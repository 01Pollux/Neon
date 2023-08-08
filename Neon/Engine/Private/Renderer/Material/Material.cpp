#include <EnginePCH.hpp>
#include <Private/Renderer/Material/Material.hpp>
#include <Renderer/Material/Builder.hpp>

#include <RHI/Swapchain.hpp>
#include <RHI/GlobalDescriptors.hpp>

#include <RHI/Resource/Views/ConstantBuffer.hpp>
#include <RHI/Resource/Views/ShaderResource.hpp>
#include <RHI/Resource/Views/UnorderedAccess.hpp>
#include <RHI/Resource/Views/RenderTarget.hpp>
#include <RHI/Resource/Views/DepthStencil.hpp>
#include <RHI/Resource/Views/Sampler.hpp>

#include <Log/Logger.hpp>

namespace ranges = std::ranges;
namespace views  = std::views;

namespace Neon::Renderer
{
    Ptr<IMaterial> IMaterial::Create(
        const GenericMaterialBuilder<false>& Builder)
    {
        return Ptr<Material>{ NEON_NEW Material(Builder) };
    }

    Ptr<IMaterial> IMaterial::Create(
        const GenericMaterialBuilder<true>& Builder)
    {
        return Ptr<Material>{ NEON_NEW Material(Builder) };
    }

    //

    static void CreateDescriptorIfNeeded(
        RHI::DescriptorHeapHandle& Descriptor,
        RHI::DescriptorType        Type,
        uint32_t                   Count)
    {
        auto Allocator = RHI::IStaticDescriptorHeap::Get(Type);
        if (Count)
        {
            Descriptor = Allocator->Allocate(Count);
        }
    }

    //

    template<bool _Compute>
    void Material_CreateDescriptors(
        const GenericMaterialBuilder<_Compute>& Builder,
        Material*                               Mat)
    {
        auto& RootSignature = Builder.RootSignature();
        NEON_ASSERT(RootSignature);

        Mat->m_RootSignature = RootSignature;

        //

        // Calculate number of descriptors needed (for shared and local descriptors)
        uint32_t TableSharedResourceCount = 0;
        uint32_t TableSharedSamplerCount  = 0;
        uint32_t TableResourceCount       = 0;
        uint32_t TableSamplerCount        = 0;

        // Calculate size of constants
        uint8_t BufferSize = 0;

        for (auto& Param : RootSignature->GetParams())
        {
            switch (Param.Type)
            {
            case RHI::IRootSignature::ParamType::Constants:
            {
                Mat->m_Parameters->Entries.emplace(Param.Name, Material::ConstantEntry{
                                                                   .DataOffset = BufferSize });
                BufferSize += Param.Constants.Num32BitValues;
                break;
            }

            case RHI::IRootSignature::ParamType::Root:
            {
                auto& Root = Param.Root;

                RHI::ICommonCommandList::ViewType ViewType;
                switch (Root.Type)
                {
                case RHI::RootParameter::RootType::ConstantBuffer:
                {
                    ViewType = RHI::ICommonCommandList::ViewType::Cbv;
                    break;
                }
                case RHI::RootParameter::RootType::ShaderResource:
                {
                    ViewType = RHI::ICommonCommandList::ViewType::Srv;
                    break;
                }
                case RHI::RootParameter::RootType::UnorderedAccess:
                {
                    ViewType = RHI::ICommonCommandList::ViewType::Uav;
                    break;
                }
                default:
                {
                    NEON_ASSERT(false, "Invalid root type");
                    break;
                }
                }
                Mat->m_Parameters->Entries.emplace(Param.Name, Material::RootEntry{ .ViewType = ViewType });
                break;
            }

            case RHI::IRootSignature::ParamType::DescriptorTable:
            {
                auto& Descriptor = Param.Descriptor;
                if (Descriptor.Type == RHI::DescriptorTableParam::Sampler)
                {
                    auto& DescriptorCount = Descriptor.Instanced ? TableResourceCount : TableSharedResourceCount;

                    Material::SamplerEntry Entry{
                        .Offset    = DescriptorCount,
                        .Count     = Descriptor.Size,
                        .Instanced = Descriptor.Instanced
                    };

                    DescriptorCount += Entry.Count;
                    Entry.Descs.resize(Descriptor.Size);

                    Mat->m_Parameters->Entries.emplace(Param.Name, std::move(Entry));
                }
                else
                {
                    auto& DescriptorCount = Descriptor.Instanced ? TableResourceCount : TableSharedResourceCount;

                    Material::DescriptorEntry Entry{
                        .Offset    = DescriptorCount,
                        .Count     = Descriptor.Size,
                        .Type      = Descriptor.Type,
                        .Instanced = Descriptor.Instanced
                    };

                    DescriptorCount += Entry.Count;
                    Entry.Resources.resize(Descriptor.Size);

                    Mat->m_Parameters->Entries.emplace(Param.Name, std::move(Entry));
                }
                break;
            }
            }
        }

        if (BufferSize)
        {
            Mat->m_Parameters->ConstantData = std::make_unique<uint8_t[]>(BufferSize);
        }

        Mat->m_SharedDescriptors = std::make_shared<Material::UnqiueDescriptorHeapHandle>(TableSharedResourceCount, TableSharedSamplerCount);
        Mat->m_LocalDescriptors  = Material::UnqiueDescriptorHeapHandle(TableResourceCount, TableSamplerCount);
    }

    //

    template<bool _Compute>
    void Material_CreatePipelineState(
        const GenericMaterialBuilder<_Compute>& Builder,
        Material*                               Mat)
    {
        if constexpr (!_Compute)
        {
            RHI::PipelineStateBuilderG PipelineDesc{
                .RootSignature = Mat->m_RootSignature,

                .VertexShader   = Builder.VertexShader(),
                .PixelShader    = Builder.PixelShader(),
                .GeometryShader = Builder.GeometryShader(),
                .HullShader     = Builder.HullShader(),
                .DomainShader   = Builder.DomainShader(),

                .Blend        = Builder.Blend(),
                .Rasterizer   = Builder.Rasterizer(),
                .DepthStencil = Builder.DepthStencil(),

                .RTFormats = Builder.RenderTargets() |
                             views::transform([](auto& RenderTarget)
                                              { return RenderTarget.second; }) |
                             ranges::to<std::vector>(),

                .SampleMask    = Builder.SampleMask(),
                .SampleCount   = Builder.SampleCount(),
                .SampleQuality = Builder.SampleQuality(),
                .Topology      = Builder.Topology(),

                .StripCut = Builder.StripCut(),
                .DSFormat = Builder.DepthStencilFormat()
            };

            {
                auto& InputLayout = Builder.InputLayout();
                if (InputLayout)
                {
                    PipelineDesc.Input = *InputLayout;
                }
                else if (!Builder.NoVertexInput())
                {
                    PipelineDesc.VertexShader->CreateInputLayout(PipelineDesc.Input);
                }
            }

            Mat->m_PipelineState = RHI::IPipelineState::Create(PipelineDesc);
        }
        else
        {
            RHI::PipelineStateBuilderC PipelineDesc{
                .RootSignature = Mat->m_RootSignature,
                .ComputeShader = Builder.ComputeShader()
            };

            Mat->m_PipelineState = RHI::IPipelineState::Create(PipelineDesc);
        }
    }

    //

    Material::Material(
        const RenderMaterialBuilder& Builder) :
        m_Parameters(std::make_shared<LayoutEntryMap>())
    {
        Material_CreateDescriptors(
            Builder,
            this);

        Material_CreatePipelineState(
            Builder,
            this);
    }

    Material::Material(
        const ComputeMaterialBuilder& Builder) :
        m_Parameters(std::make_shared<LayoutEntryMap>())
    {
        Material_CreateDescriptors(
            Builder,
            this);

        Material_CreatePipelineState(
            Builder,
            this);
    }

    Material::Material(
        Material* Other) :
        m_SharedDescriptors(Other->m_SharedDescriptors),
        m_LocalDescriptors(Other->m_LocalDescriptors.ResourceDescriptors.Size, Other->m_LocalDescriptors.SamplerDescriptors.Size),
        m_Parameters(Other->m_Parameters)
    {
        m_RootSignature = Other->m_RootSignature;
        m_PipelineState = Other->m_PipelineState;

        //

        // Iterate descriptors and copy them over
        DescriptorHeapHandle *FromHandle, *ToHandle;
        uint32_t              Count, Offset;

        for (auto& [Name, Entry] : m_Parameters->Entries)
        {
            if (auto Descriptor = std::get_if<DescriptorEntry>(&Entry))
            {
                if (!Descriptor->Instanced)
                {
                    continue;
                }

                FromHandle = &m_LocalDescriptors.ResourceDescriptors;
                ToHandle   = &Other->m_LocalDescriptors.ResourceDescriptors;

                Offset = Descriptor->Offset;
                Count  = Descriptor->Count;
            }
            else if (auto Sampler = std::get_if<SamplerEntry>(&Entry))
            {
                if (!Sampler->Instanced)
                {
                    continue;
                }

                FromHandle = &m_LocalDescriptors.SamplerDescriptors;
                ToHandle   = &Other->m_LocalDescriptors.SamplerDescriptors;

                Offset = Sampler->Offset;
                Count  = Sampler->Count;
            }
            else
            {
                continue;
            }

            FromHandle->Heap->Copy(
                FromHandle->Offset,
                RHI::IDescriptorHeap::CopyInfo{
                    .Descriptor = ToHandle->GetCpuHandle(Offset),
                    .CopySize   = Count });
        }
    }

    Ptr<IMaterial> Material::CreateInstance()
    {
        return Ptr<IMaterial>(NEON_NEW Material(this));
    }

    RHI::IDescriptorHeap::CopyInfo Material::GetDescriptorParam(
        const StringU8& ParamName) const
    {
        RHI::IDescriptorHeap::CopyInfo Result{};

        auto Param = m_Parameters->Entries.find(ParamName);
        if (Param != m_Parameters->Entries.end())
        {
            if (auto Descriptor = std::get_if<DescriptorEntry>(&Param->second))
            {
                auto& Handle = Descriptor->Instanced ? m_LocalDescriptors.ResourceDescriptors : m_SharedDescriptors->ResourceDescriptors;

                Result = {
                    .Descriptor = Handle.GetCpuHandle(Descriptor->Offset),
                    .CopySize   = Descriptor->Count
                };
            }
            else if (auto Sampler = std::get_if<SamplerEntry>(&Param->second))
            {
                auto& Handle = Sampler->Instanced ? m_LocalDescriptors.SamplerDescriptors : m_SharedDescriptors->SamplerDescriptors;

                Result = {
                    .Descriptor = Handle.GetCpuHandle(Sampler->Offset),
                    .CopySize   = Sampler->Count
                };
            }
        }

#ifdef NEON_DEBUG
        if (!Result.CopySize)
        {
            NEON_WARNING("Material", "Material parameter '{}' not found", ParamName);
        }
#endif

        return Result;
    }

    //

    const Ptr<RHI::IRootSignature>& IMaterial::GetRootSignature() const noexcept
    {
        return m_RootSignature;
    }

    const Ptr<RHI::IPipelineState>& IMaterial::GetPipelineState() const noexcept
    {
        return m_PipelineState;
    }

    void Material::ApplyAll(
        RHI::ICommonCommandList*         CommandList,
        std::span<uint32_t>              DescriptorOffsets,
        const RHI::DescriptorHeapHandle& ResourceDescriptor,
        const RHI::DescriptorHeapHandle& SamplerDescriptor) const
    {
        auto& RootSignature = GetRootSignature();

        // We will start at 1 to correctly bind root indices, (first Param always start with 0, and keep incrementing)
        uint32_t LastRootIndex         = 1;
        size_t   DescriptorOffsetIndex = 0;

        for (auto& RootParam : RootSignature->GetParams())
        {
            if (RootParam.RootIndex == LastRootIndex)
            {
                continue;
            }

            LastRootIndex = RootParam.RootIndex;
            switch (RootParam.Type)
            {
            case RHI::IRootSignature::ParamType::Constants:
            {
                auto& Constant = std::get<ConstantEntry>(m_Parameters->Entries.find(RootParam.Name)->second);

                auto DataPtr = m_Parameters->ConstantData.get() + Constant.DataOffset;
                CommandList->SetConstants(LastRootIndex, DataPtr, RootParam.Constants.Num32BitValues);
                break;
            }

            case RHI::IRootSignature::ParamType::Root:
            {
                auto& Root = std::get<RootEntry>(m_Parameters->Entries.find(RootParam.Name)->second);

                CommandList->SetResourceView(Root.ViewType, LastRootIndex, Root.Handle);
                break;
            }

            case RHI::IRootSignature::ParamType::DescriptorTable:
            {
                auto  IsSampler       = std::holds_alternative<SamplerEntry>(m_Parameters->Entries.find(RootParam.Name)->second);
                auto& DescriptorTable = IsSampler ? SamplerDescriptor : ResourceDescriptor;

                CommandList->SetDescriptorTable(LastRootIndex, DescriptorTable.GetGpuHandle(DescriptorOffsets[DescriptorOffsetIndex]));
                DescriptorOffsetIndex++;
                break;
            }

            default:
                break;
            }
        }
    }

    //

    void Material::SetResource(
        const StringU8&                Name,
        const Ptr<RHI::IGpuResource>&  Resource,
        const RHI::DescriptorViewDesc& Desc,
        uint32_t                       ArrayIndex,
        const Ptr<RHI::IGpuResource>&  UavCounter)
    {
        auto Layout = m_Parameters->Entries.find(Name);
        if (Layout == m_Parameters->Entries.end())
        {
            NEON_WARNING_TAG("Material", "Failed to find resource: {}", Name);
            return;
        }

        if (auto Descriptor = std::get_if<DescriptorEntry>(&Layout->second))
        {
            auto& Handle = Descriptor->Instanced ? m_LocalDescriptors.ResourceDescriptors : m_SharedDescriptors->ResourceDescriptors;

            uint32_t DescriptorOffset = ArrayIndex + Descriptor->Offset;

            Descriptor->Resources[ArrayIndex] = Resource;
            std::visit(
                VariantVisitor{
                    [](const std::monostate&)
                    {
                        NEON_ASSERT(false, "Invalid view type");
                    },
                    [&Handle, DescriptorOffset](
                        const RHI::CBVDesc& Desc)
                    {
                        RHI::Views::ConstantBuffer View{ Handle };
                        View.Bind(Desc, DescriptorOffset);
                    },
                    [&Handle, &Resource, DescriptorOffset](
                        const std::optional<RHI::SRVDesc>& Desc)
                    {
                        RHI::Views::ShaderResource View{ Handle };
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr, DescriptorOffset);
                    },
                    [&Handle, &Resource, &UavCounter, DescriptorOffset](
                        const std::optional<RHI::UAVDesc>& Desc)
                    {
                        RHI::Views::UnorderedAccess View{ Handle };
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr, UavCounter.get(), DescriptorOffset);
                    },
                    [&Handle, &Resource, DescriptorOffset](
                        const std::optional<RHI::RTVDesc>& Desc)
                    {
                        RHI::Views::RenderTarget View{ Handle };
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr, DescriptorOffset);
                    },
                    [&Handle, &Resource, DescriptorOffset](
                        const std::optional<RHI::DSVDesc>& Desc)
                    {
                        RHI::Views::DepthStencil View{ Handle };
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr, DescriptorOffset);
                    } },
                Desc);
        }
        else
        {
            NEON_WARNING_TAG("Material", "'{}' is not a resource", Name);
        }
    }

    void Material::SetResource(
        const StringU8&         Name,
        const RHI::SamplerDesc& Desc,
        uint32_t                ArrayIndex)
    {
        auto Layout = m_Parameters->Entries.find(Name);
        if (Layout == m_Parameters->Entries.end())
        {
            NEON_WARNING_TAG("Material", "Failed to find sampler: {}", Name);
            return;
        }

        if (auto Sampler = std::get_if<SamplerEntry>(&Layout->second))
        {
            auto& Handle = Sampler->Instanced ? m_LocalDescriptors.SamplerDescriptors : m_SharedDescriptors->SamplerDescriptors;

            uint32_t DescriptorOffset = ArrayIndex + Sampler->Offset;

            RHI::Views::Sampler View{ Handle };
            View.Bind(Desc, DescriptorOffset);
        }
        else
        {
            NEON_WARNING_TAG("Material", "'{}' is not a sampler", Name);
        }
    }

    void Material::SetConstant(
        const std::string& Name,
        const void*        Data,
        size_t             Size,
        uint32_t           Offset)
    {
        auto Layout = m_Parameters->Entries.find(Name);
        if (Layout == m_Parameters->Entries.end())
        {
            NEON_WARNING_TAG("Material", "Failed to find constant: {}", Name);
            return;
        }

        if (auto Constant = std::get_if<ConstantEntry>(&Layout->second))
        {
            std::copy_n(std::bit_cast<uint8_t*>(Data), Size, m_Parameters->ConstantData.get() + Offset);
        }
        else
        {
            NEON_WARNING_TAG("Material", "'{}' is not a constant", Name);
        }
    }

    void Material::SetResourceView(
        const std::string&     Name,
        RHI::GpuResourceHandle Handle)
    {
        auto Layout = m_Parameters->Entries.find(Name);
        if (Layout == m_Parameters->Entries.end())
        {
            NEON_WARNING_TAG("Material", "Failed to find resource view: {}", Name);
            return;
        }

        if (auto Root = std::get_if<RootEntry>(&Layout->second))
        {
            Root->Handle = Handle;
        }
        else
        {
            NEON_WARNING_TAG("Material", "'{}' is not a resource view", Name);
        }
    }

    void Material::SetResourceSize(
        const StringU8& Name,
        uint32_t        Size)
    {
        auto Layout = m_Parameters->Entries.find(Name);
        if (Layout == m_Parameters->Entries.end())
        {
            NEON_WARNING_TAG("Material", "Failed to find resource: {}", Name);
            return;
        }

        if (auto Descriptor = std::get_if<DescriptorEntry>(&Layout->second))
        {
            Descriptor->Count = Size;
        }
        else if (auto Sampler = std::get_if<Material::SamplerEntry>(&Layout->second))
        {
            Sampler->Count = Size;
        }
        else
        {
            NEON_WARNING_TAG("Material", "'{}' is not a resource nor sampler", Name);
        }
    }

    //

    Material::UnqiueDescriptorHeapHandle::UnqiueDescriptorHeapHandle(
        uint32_t ResourceDescriptorSize,
        uint32_t SamplerDescriptorSize)
    {
        CreateDescriptorIfNeeded(ResourceDescriptors, RHI::DescriptorType::ResourceView, ResourceDescriptorSize);
        CreateDescriptorIfNeeded(SamplerDescriptors, RHI::DescriptorType::Sampler, SamplerDescriptorSize);
    }

    Material::UnqiueDescriptorHeapHandle::UnqiueDescriptorHeapHandle(
        UnqiueDescriptorHeapHandle&& Other) :
        ResourceDescriptors(std::move(Other.ResourceDescriptors)),
        SamplerDescriptors(std::move(Other.SamplerDescriptors))
    {
        Other.ResourceDescriptors.Heap = nullptr;
        Other.SamplerDescriptors.Heap  = nullptr;
    }

    auto Material::UnqiueDescriptorHeapHandle::operator=(
        UnqiueDescriptorHeapHandle&& Other) -> UnqiueDescriptorHeapHandle&
    {
        if (this != &Other)
        {
            if (ResourceDescriptors)
            {
                auto Allocator = RHI::IStaticDescriptorHeap::Get(RHI::DescriptorType::ResourceView);
                Allocator->Free(ResourceDescriptors);
            }
            if (SamplerDescriptors)
            {
                auto Allocator = RHI::IStaticDescriptorHeap::Get(RHI::DescriptorType::Sampler);
                Allocator->Free(SamplerDescriptors);
            }

            ResourceDescriptors = std::move(Other.ResourceDescriptors);
            SamplerDescriptors  = std::move(Other.SamplerDescriptors);

            Other.ResourceDescriptors.Heap = nullptr;
            Other.SamplerDescriptors.Heap  = nullptr;
        }

        return *this;
    }

    Material::UnqiueDescriptorHeapHandle::~UnqiueDescriptorHeapHandle()
    {
        if (ResourceDescriptors)
        {
            auto Allocator = RHI::IStaticDescriptorHeap::Get(RHI::DescriptorType::ResourceView);
            Allocator->Free(ResourceDescriptors);
        }
        if (SamplerDescriptors)
        {
            auto Allocator = RHI::IStaticDescriptorHeap::Get(RHI::DescriptorType::Sampler);
            Allocator->Free(SamplerDescriptors);
        }
    }

    //

    void MaterialTable::Reset()
    {
        m_Materials.clear();
        m_HighestId = 0;
    }

    uint32_t MaterialTable::Append(
        IMaterial* Material)
    {
#if NEON_DEBUG
        NEON_ASSERT(Material, "Material cannot be null");
        NEON_ASSERT(m_Materials.empty() || GetFirstMaterial()->GetPipelineState() == Material->GetPipelineState(), "Material must have the same pipeline state");
#endif
        auto [Iter, WasInserted] = m_Materials.emplace(Material, m_HighestId);
        if (WasInserted)
        {
            m_HighestId++;
        }
        return Iter->second;
    }

    void MaterialTable::Bind(
        RHI::ICommonCommandList* CommandList)
    {
        std::vector<RHI::IDescriptorHeap::CopyInfo> ResourceDescriptors, SamplerDescriptors;
        std::vector<uint32_t>                       DescriptorOffsets;

        uint32_t ResourceDescriptorSize = 0, SamplerDescriptorSize = 0;
        auto     FirstMaterial = GetFirstMaterial();

        // Function helper to insert descriptor to batch and accumulate size
        auto InsertToDescriptorBatch =
            [&ResourceDescriptors,
             &ResourceDescriptorSize,
             &SamplerDescriptors,
             &SamplerDescriptorSize](const RHI::IDescriptorHeap::CopyInfo& CopyInfo, RHI::DescriptorTableParam ParamType)
        {
            auto& Container = (ParamType == RHI::DescriptorTableParam::Sampler) ? SamplerDescriptors : ResourceDescriptors;
            auto& Size      = (ParamType == RHI::DescriptorTableParam::Sampler) ? SamplerDescriptorSize : ResourceDescriptorSize;

            Container.emplace_back(CopyInfo);
            Size += CopyInfo.CopySize;
        };

        //

        // Function helper to upload descriptor to descriptor heap and return handle
        auto UploadDescriptor =
            [&ResourceDescriptors,
             &ResourceDescriptorSize,
             &SamplerDescriptors,
             &SamplerDescriptorSize](bool IsSampler) -> RHI::DescriptorHeapHandle
        {
            auto& SourceDescriptors = IsSampler ? SamplerDescriptors : ResourceDescriptors;
            auto  SourceSize        = IsSampler ? SamplerDescriptorSize : ResourceDescriptorSize;
            auto  Type              = IsSampler ? RHI::DescriptorType::Sampler : RHI::DescriptorType::ResourceView;

            if (!SourceSize)
            {
                return {};
            }

            auto ResourceTable = RHI::IFrameDescriptorHeap::Get(Type);
            auto Descriptor    = ResourceTable->Allocate(SourceSize);

            RHI::IDescriptorHeap::CopyInfo Destination{
                .Descriptor = Descriptor.GetCpuHandle(),
                .CopySize   = Descriptor.Size
            };
            RHI::IDescriptorHeap::Copy(Type, SourceDescriptors, { &Destination, 1 });
            return Descriptor;
        };

        // Function helper to upload all descriptors and return handles
        auto UploadDescriptors =
            [&UploadDescriptor]() -> std::pair<RHI::DescriptorHeapHandle, RHI::DescriptorHeapHandle>
        {
            return { UploadDescriptor(false), UploadDescriptor(true) };
        };

        //

        // All the materials shares the same root signature + pipeline state
        auto& RootSignature = FirstMaterial->GetRootSignature();

        // Bind root signature and pipeline state
        CommandList->SetRootSignature(RootSignature);
        CommandList->SetPipelineState(FirstMaterial->GetPipelineState());

        //

        DescriptorOffsets.reserve(RootSignature->GetParams().size());
        for (auto& Params = RootSignature->GetParams();
             auto& [ParamName, ParamIndex] : RootSignature->GetParamMap())
        {
            auto& Param = Params[ParamIndex];
            // Skip non-descriptor table parameters
            if (Param.Type != RHI::IRootSignature::ParamType::DescriptorTable)
            {
                continue;
            }

            auto DescriptorOffset = Param.Descriptor.Type == RHI::DescriptorTableParam::Sampler ? SamplerDescriptorSize : ResourceDescriptorSize;
            DescriptorOffsets.push_back(DescriptorOffset);

            if (Param.Descriptor.Instanced)
            {
                for (auto& CurMaterial : m_Materials)
                {
                    InsertToDescriptorBatch(CurMaterial.first->GetDescriptorParam(ParamName), Param.Descriptor.Type);
                }
            }
            else
            {
                InsertToDescriptorBatch(FirstMaterial->GetDescriptorParam(ParamName), Param.Descriptor.Type);
            }
        }

        //

        auto [ResourceDescriptor, SamplerDescriptor] = UploadDescriptors();
        validate_cast<Material*>(FirstMaterial)->ApplyAll(CommandList, DescriptorOffsets, ResourceDescriptor, SamplerDescriptor);
    }
} // namespace Neon::Renderer