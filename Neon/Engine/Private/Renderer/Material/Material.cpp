#include <EnginePCH.hpp>
#include <Private/Renderer/Material/Material.hpp>
#include <Renderer/Material/Builder.hpp>

#include <RHI/Swapchain.hpp>
#include <RHI/GlobalDescriptors.hpp>

#include <RHI/RootSignature.hpp>
#include <RHI/PipelineState.hpp>
#include <RHI/Commands/List.hpp>

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
        RHI::RootSignatureBuilder RootSigBuilder;

        uint32_t TableResourceCount       = 0,
                 TableSharedResourceCount = 0;
        uint32_t TableSamplerCount        = 0,
                 TableSharedSamplerCount  = 0;

        struct BatchedRootEntry
        {
            Material::LayoutEntry* LayoutEntry;
            ShaderBinding          Binding;
            uint32_t               Size;

            auto operator<=>(const BatchedRootEntry& Other) const noexcept
            {
                return Binding <=> Other.Binding;
            }
        };

        using BatchedRootEntrySets    = std::set<BatchedRootEntry>;
        using BatchedRootEntryVarType = std::map<MaterialVarType, BatchedRootEntrySets>;

        using BatchedRootEntryVarTypeVisibility = std::map<RHI::ShaderVisibility, BatchedRootEntryVarType>;
        using BatchedRootEntryVisibility        = std::map<RHI::ShaderVisibility, BatchedRootEntrySets>;

        // First one is for resources, second one is for samplers
        BatchedRootEntryVarTypeVisibility BatchedDescriptorEntries[2];
        BatchedRootEntryVisibility        BatchedRootConsants;

        auto& VarMap = Builder.VarMap();

        VarMap.ForEachVariable(
            [&](
                const MaterialVariableMap::View& View) mutable
            {
                const bool IsLocal     = View.Flags().Test(EMaterialVarFlags::Instanced);
                const bool IsUnbounded = View.Flags().Test(EMaterialVarFlags::Unbounded);

                switch (View.Type())
                {
                case MaterialVarType::Buffer:
                case MaterialVarType::Resource:
                case MaterialVarType::RWResource:
                case MaterialVarType::Sampler:
                {
                    const bool IsSampler   = View.Type() == MaterialVarType::Sampler;
                    size_t     BatchIndex  = IsSampler ? 1 : 0;
                    auto&      Descriptors = BatchedDescriptorEntries[BatchIndex][View.Visibility()][View.Type()];

                    // Shared resources are allocated in a limited descriptor space
                    // while unified resources are allocated in a per-material descriptor space
                    const uint32_t DescriptorSize        = IsUnbounded ? Material::UnboundedTableSize : View.ArraySize();
                    const uint32_t UnifiedDescriptorSize = (IsUnbounded || IsLocal) ? Material::UnboundedTableSize : View.ArraySize();

                    auto& DescriptorCount =
                        IsSampler ? (IsLocal ? TableSamplerCount : TableSharedSamplerCount)
                                  : (IsLocal ? TableResourceCount : TableSharedResourceCount);

                    auto& LayoutEntry = Mat->m_Descriptor->Entries[View.Name()];
                    if (IsSampler)
                    {
                        LayoutEntry.Entry = Material::SamplerEntry{
                            .Offset = DescriptorCount,
                            .Count  = DescriptorSize
                        };
                    }
                    else
                    {
                        LayoutEntry.Entry = Material::DescriptorEntry{
                            .Offset = DescriptorCount,
                            .Count  = DescriptorSize
                        };
                    }
                    LayoutEntry.IsInstanced = IsLocal;

                    // If this is a local descriptor heap, we will allocate for each resource array's size
                    // else we will allocate for the maximum array size if its unbounded or the array size if its bounded
                    DescriptorCount += DescriptorSize;

                    Descriptors.insert(
                        BatchedRootEntry{
                            .LayoutEntry = &LayoutEntry,
                            .Binding     = View.Binding(),
                            .Size        = UnifiedDescriptorSize });

                    break;
                }

                case MaterialVarType::Constant:
                {
                    auto& LayoutEntry       = Mat->m_Descriptor->Entries[View.Name()];
                    LayoutEntry.IsInstanced = false;
                    LayoutEntry.Entry       = Material::ConstantEntry{
                              .Data = std::vector<uint32_t>(View.ArraySize(), 0)
                    };

                    BatchedRootConsants[View.Visibility()].insert(
                        BatchedRootEntry{
                            .LayoutEntry = &LayoutEntry,
                            .Binding     = View.Binding(),
                            .Size        = View.ArraySize() });
                    break;
                }

                default:
                    std::unreachable();
                };
            });

        VarMap.ForEachStaticSampler(
            [&RootSigBuilder](
                const RHI::StaticSamplerDesc& Desc)
            {
                RootSigBuilder.AddSampler(Desc);
            });

        //

        // Merge batched root constants

        // Per shader loop
        for (auto& [Visibility, RootConstantSets] : BatchedRootConsants)
        {
            // Per root constant loop
            for (auto& [Layout, Binding, Size] : RootConstantSets)
            {
                RootSigBuilder.Add32BitConstants(Binding.Register, Binding.Space, Size, Visibility);
                Mat->m_Descriptor->RootParams.emplace_back(Material::ConstantEntry::Root{
                    std::get_if<Material::ConstantEntry>(&Layout->Entry) });
            }
        }

        // We will be using a contiguous descriptor table for all resources across all shaders
        // so we need to merge all the descriptor entries into a single table
        uint32_t DescriptorOffset = 0;

        // Merge batched descriptors
        // Per batch loop (resources and samplers)
        bool IsSampler = false;
        static_assert(std::size(BatchedDescriptorEntries) == 2, "Only resource and sampler batch are possible");
        for (auto& Entries : BatchedDescriptorEntries)
        {
            // Reset root param index for each batch
            // The descriptor is contigious therefore we will accumulate the offset by the size of each entry
            DescriptorOffset = 0;

            // Per shader loop
            for (auto& [Visibility, DescriptorEntries] : Entries)
            {
                if (IsSampler)
                {
                    Mat->m_Descriptor->RootParams.emplace_back(Material::SamplerEntry::Root{ DescriptorOffset });
                }
                else
                {
                    Mat->m_Descriptor->RootParams.emplace_back(Material::DescriptorEntry::Root{ DescriptorOffset });
                }

                RHI::RootDescriptorTable Table;

                // Per descriptor type loop
                for (auto& [Type, Descriptors] : DescriptorEntries)
                {
                    // Per descriptor loop
                    for (auto& [Layout, Binding, Size] : Descriptors)
                    {
                        RHI::MRootDescriptorTableFlags Flags;
                        Flags.Set(RHI::ERootDescriptorTableFlags::Descriptor_Volatile);
                        Flags.Set(RHI::ERootDescriptorTableFlags::Data_Static_While_Execute);

                        switch (Type)
                        {
                        case MaterialVarType::Buffer:
                            Table.AddCbvRange(Binding.Register, Binding.Space, Size, std::move(Flags));
                            break;

                        case MaterialVarType::Resource:
                            Table.AddSrvRange(Binding.Register, Binding.Space, Size, std::move(Flags));
                            break;

                        case MaterialVarType::RWResource:
                            Table.AddUavRange(Binding.Register, Binding.Space, Size, std::move(Flags));
                            break;

                        case MaterialVarType::Sampler:
                            Table.AddSamplerRange(Binding.Register, Binding.Space, Size, std::move(Flags));
                            break;
                        }

                        DescriptorOffset += Size;
                    }
                }
                RootSigBuilder.AddDescriptorTable(std::move(Table), Visibility);
            }
            IsSampler = true;
        }

        //

        if constexpr (_Compute)
        {
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyVSAccess, true);
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyHSAccess, true);
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyDSAccess, true);
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyGSAccess, true);
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyPSAccess, true);
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyAmpAcess, true);
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyMeshAccess, true);
        }
        else
        {
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::AllowInputLayout, !Builder.NoVertexInput());

            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyVSAccess, !Builder.VertexShader());
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyHSAccess, !Builder.HullShader());
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyDSAccess, !Builder.DomainShader());
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyGSAccess, !Builder.GeometryShader());
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyPSAccess, !Builder.PixelShader());
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyAmpAcess, true);
            RootSigBuilder.SetFlags(RHI::ERootSignatureBuilderFlags::DenyMeshAccess, true);
        }

        Mat->m_RootSignature = RHI::IRootSignature::Create(RootSigBuilder);

        //

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
                                              { return RenderTarget.second.second; }) |
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
        m_Descriptor(std::make_shared<MaterialDescriptor>())
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
        m_Descriptor(std::make_shared<MaterialDescriptor>())
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
        m_Descriptor(Other->m_Descriptor)
    {
        m_RootSignature = Other->m_RootSignature;
        m_PipelineState = Other->m_PipelineState;
    }

    Ptr<IMaterial> Material::CreateInstance()
    {
        return Ptr<IMaterial>(NEON_NEW Material(this));
    }

    //

    void Material::GetDescriptorTable(
        bool                       Local,
        RHI::DescriptorHeapHandle* OutResourceDescriptor,
        RHI::DescriptorHeapHandle* OutSamplerDescriptor) const
    {
        if (OutResourceDescriptor)
        {
            *OutResourceDescriptor = Local ? m_LocalDescriptors.ResourceDescriptors : m_SharedDescriptors->ResourceDescriptors;
        }
        if (OutSamplerDescriptor)
        {
            *OutSamplerDescriptor = Local ? m_LocalDescriptors.SamplerDescriptors : m_SharedDescriptors->SamplerDescriptors;
        }
    }

    void Material::GetUsedDescriptors(
        bool                                         Local,
        std::vector<RHI::IDescriptorHeap::CopyInfo>& OutResourceDescriptors,
        std::vector<RHI::IDescriptorHeap::CopyInfo>& OutSamplerDescriptors) const
    {
        auto& ResourceDescriptors = Local ? m_LocalDescriptors.ResourceDescriptors : m_SharedDescriptors->ResourceDescriptors;
        auto& SamplerDescriptors  = Local ? m_LocalDescriptors.SamplerDescriptors : m_SharedDescriptors->SamplerDescriptors;

        uint32_t SizeOfSharedResource = m_SharedDescriptors->ResourceDescriptors.Size;
        uint32_t SizeOfSharedSampler  = m_SharedDescriptors->SamplerDescriptors.Size;

        OutResourceDescriptors.reserve(ResourceDescriptors.Size);
        OutSamplerDescriptors.reserve(SamplerDescriptors.Size);

        for (auto& [Entry, IsInstanced] : m_Descriptor->Entries | views::values)
        {
            if (Local)
            {
                if (!IsInstanced)
                {
                    continue;
                }
            }
            else if (IsInstanced)
            {
                continue;
            }

            std::visit(
                VariantVisitor{
                    [&](const Material::DescriptorEntry& Descriptor)
                    {
                        OutResourceDescriptors.push_back({ ResourceDescriptors.GetCpuHandle(Descriptor.Offset), Descriptor.Count });
                    },
                    [&](const Material::SamplerEntry& Sampler)
                    {
                        OutSamplerDescriptors.push_back({ ResourceDescriptors.GetCpuHandle(Sampler.Offset), Sampler.Count });
                    },
                    [&](const Material::ConstantEntry&) {} },
                Entry);
        }
    }

    const Ptr<RHI::IRootSignature>& IMaterial::GetRootSignature() const noexcept
    {
        return m_RootSignature;
    }

    const Ptr<RHI::IPipelineState>& IMaterial::GetPipelineState() const noexcept
    {
        return m_PipelineState;
    }

    void Material::ApplyAll(
        RHI::IGraphicsCommandList*       CommandList,
        const RHI::DescriptorHeapHandle& ResourceDescriptor,
        const RHI::DescriptorHeapHandle& SamplerDescriptor) const
    {
        uint32_t SizeOfSharedResource = m_SharedDescriptors->ResourceDescriptors.Size;
        uint32_t SizeOfSharedSampler  = m_SharedDescriptors->SamplerDescriptors.Size;

        for (uint32_t i = 0; i < uint32_t(m_Descriptor->RootParams.size()); i++)
        {
            auto& Param = m_Descriptor->RootParams[i];
            std::visit(
                VariantVisitor{
                    [&](const Material::ConstantEntry::Root& Constant)
                    {
                        auto& Data = Constant.Entry->Data;
                        CommandList->SetConstants(i, Data.data(), Data.size());
                    },
                    [&](const Material::DescriptorEntry::Root& Descriptor)
                    {
                        CommandList->SetDescriptorTable(i, ResourceDescriptor.GetGpuHandle(Descriptor.Offset));
                    },
                    [&](const Material::SamplerEntry::Root& Sampler)
                    {
                        CommandList->SetDescriptorTable(i, SamplerDescriptor.GetGpuHandle(Sampler.Offset));
                    },
                },
                Param);
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
        auto Layout = m_Descriptor->Entries.find(Name);
        if (Layout == m_Descriptor->Entries.end())
        {
            NEON_WARNING_TAG("Material", "Failed to find resource: {}", Name);
            return;
        }

        if (auto DescriptorEntry = std::get_if<Material::DescriptorEntry>(&Layout->second.Entry))
        {
            auto& Handle = Layout->second.IsInstanced ? m_LocalDescriptors.ResourceDescriptors : m_SharedDescriptors->ResourceDescriptors;

            uint32_t DescriptorOffset = ArrayIndex + DescriptorEntry->Offset;

            DescriptorEntry->Resources[DescriptorOffset] = Resource;
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
        auto Layout = m_Descriptor->Entries.find(Name);
        if (Layout == m_Descriptor->Entries.end())
        {
            NEON_WARNING_TAG("Material", "Failed to find sampler: {}", Name);
            return;
        }

        if (auto SamplerEntry = std::get_if<Material::SamplerEntry>(&Layout->second.Entry))
        {
            auto& Handle = Layout->second.IsInstanced ? m_LocalDescriptors.SamplerDescriptors : m_SharedDescriptors->SamplerDescriptors;

            uint32_t DescriptorOffset = ArrayIndex + SamplerEntry->Offset;

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
        auto Layout = m_Descriptor->Entries.find(Name);
        if (Layout == m_Descriptor->Entries.end())
        {
            NEON_WARNING_TAG("Material", "Failed to find constant: {}", Name);
            return;
        }

        if (auto ConstantEntry = std::get_if<Material::ConstantEntry>(&Layout->second.Entry))
        {
            auto& Handle = Layout->second.IsInstanced ? m_LocalDescriptors.SamplerDescriptors : m_SharedDescriptors->SamplerDescriptors;
            std::copy_n(std::bit_cast<uint8_t*>(Data), Size, std::bit_cast<uint8_t*>(ConstantEntry->Data.data()) + Offset);
        }
        else
        {
            NEON_WARNING_TAG("Material", "'{}' is not a constant", Name);
        }
    }

    void Material::SetResourceSize(
        const StringU8& Name,
        uint32_t        Size)
    {
        auto Layout = m_Descriptor->Entries.find(Name);
        if (Layout == m_Descriptor->Entries.end())
        {
            NEON_WARNING_TAG("Material", "Failed to find resource: {}", Name);
            return;
        }

        NEON_ASSERT(!Layout->second.IsInstanced, "Only non-instanced resources can be resized");

        if (auto DescriptorEntry = std::get_if<Material::DescriptorEntry>(&Layout->second.Entry))
        {
            DescriptorEntry->Count = Size;
        }
        else if (auto SamplerEntry = std::get_if<Material::SamplerEntry>(&Layout->second.Entry))
        {
            SamplerEntry->Count = Size;
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

    Material::UnqiueDescriptorHeapHandle::UnqiueDescriptorHeapHandle(UnqiueDescriptorHeapHandle&&)                    = default;
    auto Material::UnqiueDescriptorHeapHandle::operator=(UnqiueDescriptorHeapHandle&&) -> UnqiueDescriptorHeapHandle& = default;

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

    MaterialBinder::MaterialBinder(std::span<IMaterial*> Materials) :
        m_Materials(Materials)
    {
#if NEON_DEBUG
        NEON_ASSERT(!m_Materials.empty(), "Materials are empty");
        auto FirstMaterial = m_Materials[0];
        for (auto Mat : m_Materials)
        {
            NEON_ASSERT(Mat, "Material is null");
            NEON_ASSERT(FirstMaterial->GetPipelineState() == Mat->GetPipelineState(), "Materials have different pipeline states");
        }
#endif
    }

    void MaterialBinder::Bind(
        RHI::IGraphicsCommandList* CommandList)
    {
        CommandList->SetRootSignature(m_Materials[0]->GetRootSignature());
        CommandList->SetPipelineState(m_Materials[0]->GetPipelineState());
    }

    void MaterialBinder::BindParams(
        RHI::IGraphicsCommandList* CommandList)
    {
        std::vector<RHI::IDescriptorHeap::CopyInfo> ResourceDescriptors, SamplerDescriptors;

        //

        auto UploadDescriptor =
            [&](bool IsSampler) -> RHI::DescriptorHeapHandle
        {
            auto& SourceDescriptors = IsSampler ? SamplerDescriptors : ResourceDescriptors;
            auto  Type              = IsSampler ? RHI::DescriptorType::Sampler : RHI::DescriptorType::ResourceView;

            if (SourceDescriptors.empty())
            {
                return {};
            }

            uint32_t DescriptorSize = 0;
            for (auto& Descriptor : SourceDescriptors)
            {
                DescriptorSize += Descriptor.CopySize;
            }

            auto& Descriptors = IsSampler ? SamplerDescriptors : ResourceDescriptors;

            auto ResourceTable = RHI::IFrameDescriptorHeap::Get(Type);
            auto Descriptor    = ResourceTable->Allocate(DescriptorSize);

            RHI::IDescriptorHeap::CopyInfo Destination{
                .Descriptor = Descriptor.GetCpuHandle(),
                .CopySize   = Descriptor.Size
            };
            RHI::IDescriptorHeap::Copy(Type, Descriptors, { &Destination, 1 });

            return Descriptor;
        };

        auto UploadDescriptors =
            [&]() -> std::pair<RHI::DescriptorHeapHandle, RHI::DescriptorHeapHandle>
        {
            return { UploadDescriptor(false), UploadDescriptor(true) };
        };

        //

        static std::mt19937 mt_engine;

        // Save shared descriptors
        m_Materials[0]->GetUsedDescriptors(false, ResourceDescriptors, SamplerDescriptors);

        // Save local descriptors
        for (auto Instance : m_Materials)
        {
            Instance->GetUsedDescriptors(true, ResourceDescriptors, SamplerDescriptors);
        }

        // Allocate frame descriptors and upload to it
        auto [ResourceDescriptor, SamplerDescriptor] = UploadDescriptors();
        m_Materials[0]->ApplyAll(CommandList, ResourceDescriptor, SamplerDescriptor);
    }

    void MaterialBinder::BindAll(
        RHI::IGraphicsCommandList* CommandList)
    {
        Bind(CommandList);
        BindParams(CommandList);
    }

    //

    void MaterialTable::Reset()
    {
        m_Materials.clear();
    }

    int MaterialTable::Append(
        IMaterial* Material)
    {
        for (int i = 0; i < m_Materials.size(); i++)
        {
            if (m_Materials[i] == Material)
            {
                return i;
            }
        }
        m_Materials.push_back(Material);
        return int(m_Materials.size() - 1);
    }
} // namespace Neon::Renderer