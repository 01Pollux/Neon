#include <EnginePCH.hpp>
#include <Private/Renderer/Material/Material.hpp>
#include <Renderer/Material/Builder.hpp>

#include <RHI/Swapchain.hpp>
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
        m_Parameters(std::make_shared<LayoutEntryMap>()),
        m_IsCompute(false)
    {
        Material_CreateDescriptors(
            Builder,
            this);

        // TODO: Rework material pipeline state creation
        // Material_CreatePipelineState(
        //     Builder,
        //     this);
    }

    Material::Material(
        const ComputeMaterialBuilder& Builder) :
        m_Parameters(std::make_shared<LayoutEntryMap>()),
        m_IsCompute(true)
    {
        Material_CreateDescriptors(
            Builder,
            this);

        // TODO: Rework material pipeline state creation
        // Material_CreatePipelineState(
        //    Builder,
        //    this);
    }

    Material::Material(
        Material* Other) :
        m_SharedDescriptors(Other->m_SharedDescriptors),
        m_LocalDescriptors(Other->m_LocalDescriptors.ResourceDescriptors.Size, Other->m_LocalDescriptors.SamplerDescriptors.Size),
        m_Parameters(Other->m_Parameters),
        m_IsCompute(Other->m_IsCompute)
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
                // Get either local or shared descriptor handle
                auto& Handle = Descriptor->Instanced ? m_LocalDescriptors.ResourceDescriptors : m_SharedDescriptors->ResourceDescriptors;

                // Check if we have all resources initialized
                // If not ignore the descriptor (won't be copied if possible)
                if (!ranges::any_of(Descriptor->Resources, [](auto& Resource)
                                    { return !Resource; }))
                {
                    Result = {
                        .Descriptor = Handle.GetCpuHandle(Descriptor->Offset),
                        .CopySize   = Descriptor->Count
                    };
                }
            }
            else if (auto Sampler = std::get_if<SamplerEntry>(&Param->second))
            {
                // Get either local or shared descriptor handle
                auto& Handle = Sampler->Instanced ? m_LocalDescriptors.SamplerDescriptors : m_SharedDescriptors->SamplerDescriptors;

                // Check if we have all samplers initialized
                // If not ignore the descriptor (won't be copied if possible)
                if (!ranges::any_of(Sampler->Descs, [](auto& Desc)
                                    { return !Desc.has_value(); }))
                {
                    Result = {
                        .Descriptor = Handle.GetCpuHandle(Sampler->Offset),
                        .CopySize   = Sampler->Count
                    };
                }
            }
        }

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

    void Material::Apply(
        RHI::ICommandList* CommandList)
    {
        MaterialTable::ApplyOne(this, CommandList);
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
                    [](std::monostate) {
                    },
                    [&Handle, DescriptorOffset](
                        const RHI::CBVDesc& Desc)
                    {
                        if (Desc.Resource.Value)
                        {
                            RHI::Views::ConstantBuffer View{ Handle };
                            View.Bind(Desc, DescriptorOffset);
                        }
                    },
                    [&Handle, &Resource, DescriptorOffset](
                        const RHI::SRVDescOpt& Desc)
                    {
                        if (Resource || Desc.has_value())
                        {
                            RHI::Views::ShaderResource View{ Handle };
                            View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr, DescriptorOffset);
                        }
                    },
                    [&Handle, &Resource, &UavCounter, DescriptorOffset](
                        const RHI::UAVDescOpt& Desc)
                    {
                        if (Resource || Desc.has_value())
                        {
                            RHI::Views::UnorderedAccess View{ Handle };
                            View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr, UavCounter.get(), DescriptorOffset);
                        }
                    },
                    [&Handle, &Resource, DescriptorOffset](
                        const RHI::RTVDescOpt& Desc)
                    {
                        if (Resource || Desc.has_value())
                        {
                            RHI::Views::RenderTarget View{ Handle };
                            View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr, DescriptorOffset);
                        }
                    },
                    [&Handle, &Resource, DescriptorOffset](
                        const RHI::DSVDescOpt& Desc)
                    {
                        if (Resource || Desc.has_value())
                        {
                            RHI::Views::DepthStencil View{ Handle };
                            View.Bind(Resource.get(), Desc.has_value() ? &*Desc : nullptr, DescriptorOffset);
                        }
                    } },
                Desc);
        }
        else
        {
            NEON_WARNING_TAG("Material", "'{}' is not a resource", Name);
        }
    }

    void Material::SetSampler(
        const StringU8&                        Name,
        const std::optional<RHI::SamplerDesc>& Desc,
        uint32_t                               ArrayIndex)
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

            uint32_t DescriptorOffset  = ArrayIndex + Sampler->Offset;
            Sampler->Descs[ArrayIndex] = Desc;
            if (Desc)
            {
                RHI::Views::Sampler View{ Handle };
                View.Bind(*Desc, DescriptorOffset);
            }
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
        m_MaterialMap.clear();
    }

    uint32_t MaterialTable::Append(
        IMaterial* Material)
    {
#if NEON_DEBUG
        NEON_ASSERT(Material, "Material cannot be null");
        NEON_ASSERT(m_Materials.empty() || GetFirstMaterial()->GetPipelineState() == Material->GetPipelineState(), "Material must have the same pipeline state");
#endif
        auto [Iter, WasInserted] = m_MaterialMap.emplace(Material, 0);
        if (WasInserted)
        {
            Iter->second = uint32_t(m_Materials.size());
            m_Materials.emplace_back(Material);
        }
        return Iter->second;
    }

    //

    void MaterialTable::Apply(
        RHI::ICommandList* CommandList)
    {
        Apply(GetFirstMaterial(), m_Materials, CommandList);
    }

    void MaterialTable::ApplyOne(
        IMaterial*         Material,
        RHI::ICommandList* CommandList)
    {
        Apply(Material, { &Material, 1 }, CommandList);
    }

    //

    void MaterialTable::Apply(
        IMaterial*            FirstMaterial,
        std::span<IMaterial*> Materials,
        RHI::ICommandList*    CommandList)
    {
    }
} // namespace Neon::Renderer