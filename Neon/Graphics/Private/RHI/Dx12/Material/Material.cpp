#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Material/Material.hpp>
#include <Private/RHI/Dx12/RootSignature.hpp>
#include <Private/RHI/Dx12/PipelineState.hpp>
#include <RHI/Material/Builder.hpp>

#include <RHI/GlobalDescriptors.hpp>
#include <RHI/GlobalBuffer.hpp>

#include <RHI/Resource/Views/ConstantBuffer.hpp>
#include <RHI/Resource/Views/ShaderResource.hpp>
#include <RHI/Resource/Views/UnorderedAccess.hpp>
#include <RHI/Resource/Views/RenderTarget.hpp>
#include <RHI/Resource/Views/DepthStencil.hpp>
#include <RHI/Resource/Views/Sampler.hpp>

#include <Log/Logger.hpp>

namespace ranges = std::ranges;
namespace views  = std::views;

namespace Neon::RHI
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
        DescriptorHeapHandle& Descriptor,
        DescriptorType        Type,
        uint32_t              Count)
    {
        auto Allocator = IStaticDescriptorHeap::Get(Type);
        if (Count)
        {
            Descriptor = Allocator->Allocate(Count);
        }
    }

    static void InitializeDescriptorIfNeeded(
        DescriptorHeapHandle&       Destination,
        DescriptorType              Type,
        const DescriptorHeapHandle& Source)
    {
        auto Allocator = IStaticDescriptorHeap::Get(Type);
        if (Destination)
        {
            Destination->Copy(
                Destination.Offset,
                IDescriptorHeap::CopyInfo{
                    Source.GetCpuHandle(),
                    Source.Size });
        }
    }

    static void CreateAndInitializeDescriptorIfNeeded(
        DescriptorHeapHandle&       Destination,
        DescriptorType              Type,
        const DescriptorHeapHandle& Source)
    {
        CreateDescriptorIfNeeded(Destination, Type, Source.Size);
        InitializeDescriptorIfNeeded(Destination, Type, Source);
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

        // Calculate number of descriptors needed (for shared and local descriptors)
        uint32_t TableSharedResourceCount = 0;
        uint32_t TableSharedSamplerCount  = 0;
        uint32_t TableResourceCount       = 0;
        uint32_t TableSamplerCount        = 0;

        // Calculate size of constants
        uint8_t BufferSize = 0;

        for (auto& [VarName, ParamIndex] : RootSignature->GetNamedParams())
        {
            auto& Param = RootSignature->GetParams()[ParamIndex];
            boost::apply_visitor(
                VariantVisitor{
                    [&BufferSize](const IRootSignature::ParamConstant& Constant)
                    {
                        BufferSize += Constant.Num32BitValues * sizeof(uint32_t);
                    },
                    [Mat, &VarName](const IRootSignature::ParamRoot& Root)
                    {
                        CstResourceViewType ViewType;
                        switch (Root.Type)
                        {
                        case RootParameter::RootType::ConstantBuffer:
                        {
                            ViewType = CstResourceViewType::Cbv;
                            break;
                        }
                        case RootParameter::RootType::ShaderResource:
                        {
                            ViewType = CstResourceViewType::Srv;
                            break;
                        }
                        case RootParameter::RootType::UnorderedAccess:
                        {
                            ViewType = CstResourceViewType::Uav;
                            break;
                        }
                        default:
                        {
                            NEON_ASSERT(false, "Invalid root type");
                            break;
                        }
                        }
                        Mat->m_SharedParameters->SharedEntries.emplace(VarName, Material::RootEntry{ .ViewType = ViewType });
                    },
                    [&](const IRootSignature::ParamDescriptor& Descriptor)
                    {
                        const bool IsSamplerDescriptor = Descriptor.NamedRanges.begin()->second.Type == DescriptorTableParam::Sampler;
                        if (IsSamplerDescriptor)
                        {
                            auto& DescriptorCount = Descriptor.Instanced ? TableSamplerCount : TableSharedSamplerCount;

                            Material::DescriptorVariantMap SamplerMap{
                                .Offset = DescriptorCount
                            };
                            for (auto& [Name, Range] : Descriptor.NamedRanges)
                            {
                                Material::SamplerEntry Entry{
                                    .Offset = DescriptorCount,
                                    .Count  = Range.Size,
                                };

                                DescriptorCount += Range.Size;
                                Entry.Descs.resize(Range.Size);

                                bool WasInserted = SamplerMap.Entries.emplace(Name, std::move(Entry)).second;
                                NEON_ASSERT(WasInserted, "Duplicate sampler variable");
                            }

                            bool WasInserted = false;
                            if (Descriptor.Instanced)
                            {
                                WasInserted = Mat->m_LocalParameters.LocalEntries.emplace(VarName, std::move(SamplerMap)).second;
                            }
                            else
                            {
                                WasInserted = Mat->m_SharedParameters->SharedEntries.emplace(VarName, std::move(SamplerMap)).second;
                            }
                            NEON_ASSERT(WasInserted, "Duplicate descriptor sampler table");
                        }
                        else
                        {
                            auto& DescriptorCount = Descriptor.Instanced ? TableResourceCount : TableSharedResourceCount;

                            Material::DescriptorVariantMap DescriptorMap{
                                .Offset = DescriptorCount
                            };

                            for (auto& [Name, Range] : Descriptor.NamedRanges)
                            {
                                Material::DescriptorEntry Entry{
                                    .Offset = DescriptorCount,
                                    .Count  = Range.Size,
                                    .Type   = Range.Type
                                };

                                DescriptorCount += Range.Size;
                                Entry.Resources.resize(Range.Size);

                                bool WasInserted = DescriptorMap.Entries.emplace(Name, std::move(Entry)).second;
                                NEON_ASSERT(WasInserted, "Duplicate descriptor variable");
                            }

                            bool WasInserted = false;
                            if (Descriptor.Instanced)
                            {
                                WasInserted = Mat->m_LocalParameters.LocalEntries.emplace(VarName, std::move(DescriptorMap)).second;
                            }
                            else
                            {
                                WasInserted = Mat->m_SharedParameters->SharedEntries.emplace(VarName, std::move(DescriptorMap)).second;
                            }
                            NEON_ASSERT(WasInserted, "Duplicate descriptor table");
                        }
                    },
                },
                Param);

            if (BufferSize)
            {
                Mat->m_SharedParameters->ConstantData = std::make_unique<uint8_t[]>(BufferSize);
            }

            Mat->m_SharedParameters->Descriptors = Material::UnqiueDescriptorHeapHandle(TableSharedResourceCount, TableSharedSamplerCount);
            Mat->m_LocalParameters.Descriptors   = Material::UnqiueDescriptorHeapHandle(TableResourceCount, TableSamplerCount);
        }
    }

    //

    template<bool _Compute>
    void Material_CreatePipelineState(
        const GenericMaterialBuilder<_Compute>& Builder,
        Material*                               Mat)
    {
        if constexpr (!_Compute)
        {
            PipelineStateBuilderG PipelineDesc{
                .RootSignature = Mat->m_RootSignature,

                .VertexShader   = Builder.VertexShader(),
                .GeometryShader = Builder.GeometryShader(),
                .HullShader     = Builder.HullShader(),
                .DomainShader   = Builder.DomainShader(),

                .Blend        = Builder.Blend(),
                .Rasterizer   = Builder.Rasterizer(),
                .DepthStencil = Builder.DepthStencil(),

                .SampleMask    = Builder.SampleMask(),
                .SampleCount   = Builder.SampleCount(),
                .SampleQuality = Builder.SampleQuality(),
                .Topology      = Builder.Topology(),

                .StripCut = Builder.StripCut(),
                .DSFormat = EResourceFormat::D32_Float
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

            for (auto& BlendRenderTarget : PipelineDesc.Blend.RenderTargets)
            {
                BlendRenderTarget.BlendEnable = false;
                BlendRenderTarget.LogicEnable = false;
            }

            for (size_t i = 0; i < size_t(IMaterial::PipelineVariant::Count); i++)
            {
                PipelineDesc.RTFormats.clear();

                switch (IMaterial::PipelineVariant(i))
                {
                case IMaterial::PipelineVariant::RenderPass:
                {
                    PipelineDesc.PixelShader = Builder.PixelShader();
                    PipelineDesc.RTFormats.push_back(ISwapchain::Get()->GetFormat());
                    PipelineDesc.Blend.RenderTargets[0].BlendEnable = false;
                    PipelineDesc.DepthStencil.DepthWriteEnable      = true;
                    break;
                }
                case IMaterial::PipelineVariant::RenderPassTransparent:
                {
                    PipelineDesc.PixelShader = Builder.PixelShader();
                    PipelineDesc.RTFormats.push_back(ISwapchain::Get()->GetFormat());
                    PipelineDesc.Blend.RenderTargets[0].BlendEnable = true;
                    PipelineDesc.DepthStencil.DepthWriteEnable      = false;
                    break;
                }
                case IMaterial::PipelineVariant::DepthPrePass:
                {
                    PipelineDesc.PixelShader                        = nullptr;
                    PipelineDesc.Blend.RenderTargets[0].BlendEnable = false;
                    PipelineDesc.DepthStencil.DepthWriteEnable      = true;
                    break;
                }
                }
                Mat->m_PipelineStates[i] = IPipelineState::Create(PipelineDesc);
            }
        }
        else
        {
            PipelineStateBuilderC PipelineDesc{
                .RootSignature = Mat->m_RootSignature,
                .ComputeShader = Builder.ComputeShader()
            };

            Mat->m_PipelineStates[size_t(IMaterial::PipelineVariant::ComputePass)] = IPipelineState::Create(PipelineDesc);
        }
    }

    //

    Material::Material(
        const RenderMaterialBuilder& Builder)
    {
        Material_CreateDescriptors(
            Builder,
            this);

        Material_CreatePipelineState(
            Builder,
            this);
    }

    Material::Material(
        const ComputeMaterialBuilder& Builder)
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
        m_SharedParameters(Other->m_SharedParameters),
        m_LocalParameters(Other->m_LocalParameters)
    {
        m_RootSignature  = Other->m_RootSignature;
        m_PipelineStates = Other->m_PipelineStates;
    }

    Ptr<IMaterial> Material::CreateInstance()
    {
        return Ptr<IMaterial>(NEON_NEW Material(this));
    }

    bool Material::IsCompute() const noexcept
    {
        return m_IsCompute;
    }

    bool Material::IsTransparent() const noexcept
    {
        return m_IsTransparent;
    }

    void Material::SetTransparent(
        bool State) noexcept
    {
        m_IsTransparent = State;
    }

    //

    void Material::BindSharedParams(
        ICommandList* CommandList)
    {
        auto& Params = m_RootSignature->GetParams();
        for (uint32_t i = 0; i < uint32_t(Params.size()); i++)
        {
            auto& Param = Params[i];
            boost::apply_visitor(
                VariantVisitor{
                    [&](const IRootSignature::ParamConstant& Constant)
                    {
                        auto& Entry = std::get<ConstantEntry>(m_SharedParameters->SharedEntries.at(Constant.Name));
                        CommandList->SetConstants(!m_IsCompute, i, m_SharedParameters->ConstantData.get() + Entry.DataOffset);
                    },
                    [&](const IRootSignature::ParamRoot& Root)
                    {
                        auto& Entry = std::get<RootEntry>(m_SharedParameters->SharedEntries.at(Root.Name));
                        if (Entry.Handle)
                        {
                            CommandList->SetResourceView(!m_IsCompute, Entry.ViewType, i, Entry.Handle);
                        }
                    },
                    [&](const IRootSignature::ParamDescriptor& Descriptor)
                    {
                        if (!Descriptor.Instanced)
                        {
                            auto& Entry          = std::get<DescriptorVariantMap>(m_SharedParameters->SharedEntries.at(Descriptor.Name));
                            auto& DescriptorHeap = Entry.IsSampler()
                                                       ? m_SharedParameters->Descriptors.SamplerDescriptors
                                                       : m_SharedParameters->Descriptors.ResourceDescriptors;

                            CommandList->SetDynamicDescriptorTable(!m_IsCompute, i, DescriptorHeap.GetCpuHandle(Entry.Offset), Descriptor.Size, Entry.IsSampler());
                        }
                    } },
                Param);
        }
    }

    void Material::BindLocalParams(
        ICommandList* CommandList)
    {
        auto& Params = m_RootSignature->GetParams();
        for (uint32_t i = 0; i < uint32_t(Params.size()); i++)
        {
            auto& Param = Params[i];
            boost::apply_visitor(
                VariantVisitor{
                    [&](const auto&) {},
                    [&](const IRootSignature::ParamDescriptor& Descriptor)
                    {
                        if (Descriptor.Instanced)
                        {
                            auto& Entry          = m_LocalParameters.LocalEntries.at(Descriptor.Name);
                            auto& DescriptorHeap = Entry.IsSampler()
                                                       ? m_LocalParameters.Descriptors.SamplerDescriptors
                                                       : m_LocalParameters.Descriptors.ResourceDescriptors;

                            CommandList->SetDynamicDescriptorTable(!m_IsCompute, i, DescriptorHeap.GetCpuHandle(Entry.Offset), Descriptor.Size, Entry.IsSampler());
                        }
                    } },
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
        auto [GroupName, VarName] = SplitResourceName(Name);

        UnqiueDescriptorHeapHandle* DescriptorHeap = nullptr;

        auto Variant = GetDescriptorVariant(GroupName, VarName, nullptr, &DescriptorHeap);

        if (!Variant) [[unlikely]]
        {
            return;
        }

        auto Descriptor = std::get_if<DescriptorEntry>(Variant);
        if (!Descriptor)
        {
            NEON_WARNING_TAG("Material", "'{}' is not a resource", Name);
            return;
        }

        uint32_t DescriptorOffset = ArrayIndex + Descriptor->Offset;

        Descriptor->Resources[ArrayIndex] = Resource;
        std::visit(
            VariantVisitor{
                [](std::monostate) {
                },
                [DescriptorHeap, DescriptorOffset](
                    const RHI::CBVDesc& Desc)
                {
                    if (Desc.Resource.Value)
                    {
                        RHI::Views::ConstantBuffer View{ DescriptorHeap->ResourceDescriptors };
                        View.Bind(Desc, DescriptorOffset);
                    }
                },
                [DescriptorHeap, &Resource, DescriptorOffset](
                    const RHI::SRVDescOpt& Desc)
                {
                    if (Resource || Desc.has_value())
                    {
                        RHI::Views::ShaderResource View{ DescriptorHeap->ResourceDescriptors };
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr, DescriptorOffset);
                    }
                },
                [DescriptorHeap, &Resource, &UavCounter, DescriptorOffset](
                    const RHI::UAVDescOpt& Desc)
                {
                    if (Resource || Desc.has_value())
                    {
                        RHI::Views::UnorderedAccess View{ DescriptorHeap->ResourceDescriptors };
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr, UavCounter.get(), DescriptorOffset);
                    }
                },
                [DescriptorHeap, &Resource, DescriptorOffset](
                    const RHI::RTVDescOpt& Desc)
                {
                    if (Resource || Desc.has_value())
                    {
                        RHI::Views::RenderTarget View{ DescriptorHeap->ResourceDescriptors };
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr, DescriptorOffset);
                    }
                },
                [DescriptorHeap, &Resource, DescriptorOffset](
                    const RHI::DSVDescOpt& Desc)
                {
                    if (Resource || Desc.has_value())
                    {
                        RHI::Views::DepthStencil View{ DescriptorHeap->ResourceDescriptors };
                        View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr, DescriptorOffset);
                    }
                } },
            Desc);
    }

    void Material::SetSampler(
        const StringU8&         Name,
        const RHI::SamplerDesc& Desc,
        uint32_t                ArrayIndex)
    {
        auto [GroupName, VarName] = SplitResourceName(Name);

        UnqiueDescriptorHeapHandle* DescriptorHeap = nullptr;

        auto Variant = GetDescriptorVariant(GroupName, VarName, nullptr, &DescriptorHeap);

        if (!Variant) [[unlikely]]
        {
            return;
        }

        auto Sampler = std::get_if<SamplerEntry>(Variant);
        if (!Sampler)
        {
            NEON_WARNING_TAG("Material", "'{}' is not a sampler", Name);
            return;
        }

        uint32_t DescriptorOffset  = ArrayIndex + Sampler->Offset;
        Sampler->Descs[ArrayIndex] = Desc;
        if (Desc)
        {
            RHI::Views::Sampler View{ DescriptorHeap->SamplerDescriptors };
            View.Bind(Desc, DescriptorOffset);
        }
    }

    void Material::SetConstant(
        const std::string& Name,
        const void*        Data,
        size_t             Size,
        uint32_t           Offset)
    {
        auto Iter = m_SharedParameters->SharedEntries.find(Name);
        if (Iter == m_SharedParameters->SharedEntries.end())
        {
            NEON_WARNING_TAG("Material", "Failed to find constant: {}", Name);
            return;
        }

        if (auto Constant = std::get_if<ConstantEntry>(&Iter->second))
        {
            std::copy_n(std::bit_cast<uint8_t*>(Data), Size, m_SharedParameters->ConstantData.get() + Offset);
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
        auto Iter = m_SharedParameters->SharedEntries.find(Name);
        if (Iter == m_SharedParameters->SharedEntries.end())
        {
            NEON_WARNING_TAG("Material", "Failed to find resource view: {}", Name);
            return;
        }

        if (auto Root = std::get_if<RootEntry>(&Iter->second))
        {
            Root->Handle = Handle;
        }
        else
        {
            NEON_WARNING_TAG("Material", "'{}' is not a resource view", Name);
        }
    }

    void Material::SetDynamicResourceView(
        const StringU8&          Name,
        RHI::CstResourceViewType Type,
        const void*              Data,
        size_t                   Size)
    {
        const uint32_t Alignment =
            Type == RHI::CstResourceViewType::Cbv ? 256 : 1;

        using PoolBufferType = RHI::IGlobalBufferPool::BufferType;

        PoolBufferType BufferType;
        switch (Type)
        {
        case RHI::CstResourceViewType::Cbv:
        case RHI::CstResourceViewType::Srv:
            BufferType = PoolBufferType::ReadWriteGPUR;
            break;
        case RHI::CstResourceViewType::Uav:
            BufferType = PoolBufferType::ReadWriteGPURW;
            break;
        default:
            std::unreachable();
        }

        RHI::UBufferPoolHandle Buffer(
            Size,
            Alignment,
            BufferType);

        Buffer.AsUpload().Write(0, Data, Size);

        SetResourceView(
            Name,
            Buffer.GetGpuHandle());
    }

    void Material::SetResourceSize(
        const StringU8& Name,
        uint32_t        Size)
    {
        auto [GroupName, VarName] = SplitResourceName(Name);
        auto Variant              = GetDescriptorVariant(GroupName, VarName);

        if (!Variant) [[unlikely]]
        {
            return;
        }

        if (auto Descriptor = std::get_if<DescriptorEntry>(Variant))
        {
            Descriptor->Count = Size;
        }
        else if (auto Sampler = std::get_if<SamplerEntry>(Variant))
        {
            Sampler->Count = Size;
        }
        else
        {
            NEON_WARNING_TAG("Material", "'{}' is not a resource nor sampler", Name);
        }
    }

    //

    std::pair<StringU8, StringU8> Material::SplitResourceName(
        const StringU8& Name)
    {
        auto Names = std::views::split(Name, ".");

        StringU8View FirstPart(*Names.begin());
        StringU8View SecondPart;
        if (auto SecondPartIter = std::next(Names.begin()); SecondPartIter != Names.end())
        {
            SecondPart = StringU8View(*SecondPartIter);
        }
        return { StringU8(FirstPart), StringU8(SecondPart) };
    }

    auto Material::GetDescriptorVariant(
        const StringU8&              Name,
        const StringU8&              EntryName,
        DescriptorVariantMap**       VariantMap,
        UnqiueDescriptorHeapHandle** DescriptorHeap) -> DescriptorVariant*
    {
        DescriptorVariantMap* InVariantMap = nullptr;

        if (auto Iter = m_LocalParameters.LocalEntries.find(Name);
            Iter != m_LocalParameters.LocalEntries.end())
        {
            InVariantMap = &Iter->second;
            if (DescriptorHeap)
            {
                *DescriptorHeap = &m_LocalParameters.Descriptors;
            }
        }
        else if (auto Iter = m_SharedParameters->SharedEntries.find(Name);
                 Iter != m_SharedParameters->SharedEntries.end())
        {
            InVariantMap = std::get_if<DescriptorVariantMap>(&Iter->second);
            if (DescriptorHeap)
            {
                *DescriptorHeap = &m_SharedParameters->Descriptors;
            }
        }

        if (!InVariantMap) [[unlikely]]
        {
            NEON_WARNING_TAG("Material", "Failed to find resource: {}", Name);
            return nullptr;
        }

        if (VariantMap)
        {
            *VariantMap = InVariantMap;
        }

        if (auto Iter = InVariantMap->Entries.find(Name);
            Iter != InVariantMap->Entries.end())
        {
            return &Iter->second;
        }
        else
        {
            NEON_WARNING_TAG("Material", "Failed to find resource: {}", Name);
            return nullptr;
        }
    }

    //

    Material::UnqiueDescriptorHeapHandle::UnqiueDescriptorHeapHandle(
        uint32_t ResourceDescriptorSize,
        uint32_t SamplerDescriptorSize)
    {
        CreateDescriptorIfNeeded(ResourceDescriptors, DescriptorType::ResourceView, ResourceDescriptorSize);
        CreateDescriptorIfNeeded(SamplerDescriptors, DescriptorType::Sampler, SamplerDescriptorSize);
    }

    Material::UnqiueDescriptorHeapHandle::UnqiueDescriptorHeapHandle(
        const UnqiueDescriptorHeapHandle& Other)
    {
        CreateAndInitializeDescriptorIfNeeded(ResourceDescriptors, DescriptorType::ResourceView, Other.ResourceDescriptors);
        CreateAndInitializeDescriptorIfNeeded(SamplerDescriptors, DescriptorType::Sampler, Other.SamplerDescriptors);
    }

    auto Material::UnqiueDescriptorHeapHandle::operator=(
        const UnqiueDescriptorHeapHandle& Other)
        -> UnqiueDescriptorHeapHandle&
    {
        if (this != &Other)
        {
            Release();

            CreateAndInitializeDescriptorIfNeeded(ResourceDescriptors, DescriptorType::ResourceView, Other.ResourceDescriptors);
            CreateAndInitializeDescriptorIfNeeded(SamplerDescriptors, DescriptorType::Sampler, Other.SamplerDescriptors);
        }

        return *this;
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
        UnqiueDescriptorHeapHandle&& Other)
        -> UnqiueDescriptorHeapHandle&
    {
        if (this != &Other)
        {
            Release();

            ResourceDescriptors = std::move(Other.ResourceDescriptors);
            SamplerDescriptors  = std::move(Other.SamplerDescriptors);

            Other.ResourceDescriptors.Heap = nullptr;
            Other.SamplerDescriptors.Heap  = nullptr;
        }

        return *this;
    }

    Material::UnqiueDescriptorHeapHandle::~UnqiueDescriptorHeapHandle()
    {
        Release();
    }

    void Material::UnqiueDescriptorHeapHandle::Release()
    {
        if (ResourceDescriptors)
        {
            auto Allocator = IStaticDescriptorHeap::Get(DescriptorType::ResourceView);
            Allocator->Free(ResourceDescriptors);
        }
        if (SamplerDescriptors)
        {
            auto Allocator = IStaticDescriptorHeap::Get(DescriptorType::Sampler);
            Allocator->Free(SamplerDescriptors);
        }
    }
} // namespace Neon::RHI