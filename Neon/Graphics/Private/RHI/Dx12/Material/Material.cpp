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
    void Material_CreatePipelineState(
        const GenericMaterialBuilder<_Compute>& Builder,
        Material*                               Mat)
    {
        if constexpr (!_Compute)
        {
            PipelineStateBuilderG PipelineDesc{
                .RootSignature = IRootSignature::Get(RSCommon::Type::Material),

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
                    PipelineDesc.DepthStencil.DepthCmpFunc          = ECompareFunc::Equal;
                    PipelineDesc.DepthStencil.DepthWriteEnable      = false;
                    break;
                }
                case IMaterial::PipelineVariant::RenderPassTransparent:
                {
                    PipelineDesc.PixelShader = Builder.PixelShader();
                    PipelineDesc.RTFormats.push_back(ISwapchain::Get()->GetFormat());
                    PipelineDesc.Blend.RenderTargets[0].BlendEnable = true;
                    PipelineDesc.DepthStencil.DepthCmpFunc          = ECompareFunc::Equal;
                    PipelineDesc.DepthStencil.DepthWriteEnable      = false;
                    break;
                }
                case IMaterial::PipelineVariant::DepthPrePass:
                {
                    PipelineDesc.PixelShader                        = nullptr;
                    PipelineDesc.Blend.RenderTargets[0].BlendEnable = false;
                    PipelineDesc.DepthStencil.DepthCmpFunc          = ECompareFunc::LessEqual;
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
                .RootSignature = IRootSignature::Get(RSCommon::Type::Material),
                .ComputeShader = Builder.ComputeShader()
            };

            Mat->m_PipelineStates[size_t(IMaterial::PipelineVariant::ComputePass)] = IPipelineState::Create(PipelineDesc);
        }
    }

    //

    void Material_CreateParameterBlock(
        const MaterialVarBuilder& VarBuilder,
        Material*                 Mat)
    {
        auto InsertToParameters = [](Material::Blackboard&                       Blackboard,
                                     Structured::LayoutBuilder&                  Builder,
                                     const MaterialVarBuilder::LayoutDescriptor& Layout)
        {
            auto     Internal = Builder.Append(Structured::Type::Struct, "_Internal");
            uint32_t Offset   = 0;
            for (auto& Resource : Layout.Resources)
            {
                Internal.Append(Structured::Type::Int, Resource);
                Blackboard.Entries.emplace(
                    Resource,
                    Material::ResourceEntry{
                        .Offset = Offset++ });
            }

            Offset = 0;
            for (auto& Sampler : Layout.Samplers)
            {
                Internal.Append(Structured::Type::Int, Sampler);
                Blackboard.Entries.emplace(
                    Sampler,
                    Material::SamplerEntry{
                        .Offset = Offset++ });
            }
        };

        // Local parameters
        {
            // Align to 4 bytes
            constexpr size_t MemberAlignement = 4;
            constexpr size_t BufferAlignement = ShaderResourceAlignement;

            auto& Layout = VarBuilder.LocalLayout();

            Structured::LayoutBuilder Builder(MemberAlignement);
            InsertToParameters(Mat->m_LocalParameters, Builder, Layout);

            Mat->m_LocalParameters.Buffer.Struct.Append(true, BufferAlignement, Builder);
            Mat->m_LocalParameters.Buffer.Struct.Append(true, BufferAlignement, Layout.LayoutBuilder);
            Mat->m_LocalParameters.Buffer.Data = UBufferPoolHandle{ Mat->m_LocalParameters.Buffer.Struct.GetSize(), ShaderResourceAlignement, IGlobalBufferPool::BufferType::ReadWriteGPUR };
            if (Mat->m_LocalParameters.Buffer.Data)
            {
                Mat->m_LocalParameters.Buffer.MappedData = Mat->m_LocalParameters.Buffer.Data.AsUpload().Map() + Mat->m_LocalParameters.Buffer.Data.Offset;
            }
        }

        // Shared parameters
        {
            // Align to 16 bytes
            constexpr size_t MemberAlignement = 16;
            constexpr size_t BufferAlignement = ConstantBufferAlignement;

            auto& Layout = VarBuilder.SharedLayout();

            Structured::LayoutBuilder Builder(MemberAlignement);
            InsertToParameters(*Mat->m_SharedParameters, Builder, Layout);

            Mat->m_SharedParameters->Buffer.Struct.Append(true, BufferAlignement, Builder);
            Mat->m_SharedParameters->Buffer.Struct.Append(true, BufferAlignement, Layout.LayoutBuilder);
            Mat->m_SharedParameters->Buffer.Data = UBufferPoolHandle{ Mat->m_SharedParameters->Buffer.Struct.GetSize(), ConstantBufferAlignement, IGlobalBufferPool::BufferType::ReadWriteGPUR };
            if (Mat->m_SharedParameters->Buffer.Data)
            {
                Mat->m_SharedParameters->Buffer.MappedData = Mat->m_SharedParameters->Buffer.Data.AsUpload().Map() + Mat->m_SharedParameters->Buffer.Data.Offset;
            }
        }
    }

    //

    Material::Material(
        const RenderMaterialBuilder& Builder)
    {
        Material_CreateParameterBlock(
            Builder.VarBuilder(),
            this);
        Material_CreatePipelineState(
            Builder,
            this);
    }

    Material::Material(
        const ComputeMaterialBuilder& Builder)
    {
        Material_CreatePipelineState(
            Builder,
            this);
    }

    Material::Material(
        Material* Other) :
        m_SharedParameters(Other->m_SharedParameters),
        m_LocalParameters(Other->m_LocalParameters)
    {
        m_PipelineStates = Other->m_PipelineStates;
    }

    Material::~Material()
    {
    }

    //

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

    static void Material_UpdateParameterBlock(
        const Material::Blackboard& Board)
    {
        if (!Board.Buffer.Data)
        {
            return;
        }

        auto InternalView = Board.Buffer.Struct["_Internal"];
        for (auto& [Name, Entry] : Board.Entries)
        {
            int& Offset = Structured::BufferView{ Board.Buffer.MappedData, InternalView[Name], 0 };
            std::visit(
                VariantVisitor{
                    [&Offset](const Material::ResourceEntry& Desc)
                    {
                        Offset = Desc.Resource || Desc.Desc.index() ? Desc.Offset : -1;
                    },
                    [&Offset](const Material::SamplerEntry& Desc)
                    {
                        Offset = Desc.Desc ? Desc.Offset : -1;
                    } },
                Entry);
        }
    }

    //

    void Material::ReallocateShared()
    {
        Material_UpdateParameterBlock(*m_SharedParameters);
    }

    void Material::ReallocateLocal()
    {
        Material_UpdateParameterBlock(m_LocalParameters);
    }

    GpuResourceHandle Material::GetSharedBlock()
    {
        return m_SharedParameters->Buffer.Data ? m_SharedParameters->Buffer.Data.GetGpuHandle() : GpuResourceHandle{};
    }

    GpuResourceHandle Material::GetLocalBlock()
    {
        return m_LocalParameters.Buffer.Data ? m_LocalParameters.Buffer.Data.GetGpuHandle() : GpuResourceHandle{};
    }

    //

    void Material::SetResource(
        const StringU8&           Name,
        const Ptr<IGpuResource>&  Resource,
        const DescriptorViewDesc& Desc)
    {
        ResourceEntry*        Entry      = nullptr;
        DescriptorHeapHandle* Descriptor = nullptr;

        if (auto Iter = m_LocalParameters.Entries.find(Name);
            Iter != m_LocalParameters.Entries.end())
        {
            Entry      = std::get_if<ResourceEntry>(&Iter->second);
            Descriptor = &m_LocalParameters.Descriptors.ResourceDescriptors;
        }
        else if (auto Iter = m_SharedParameters->Entries.find(Name);
                 Iter != m_SharedParameters->Entries.end())
        {
            Entry      = std::get_if<ResourceEntry>(&Iter->second);
            Descriptor = &m_SharedParameters->Descriptors.ResourceDescriptors;
        }

        if (!Entry)
        {
            NEON_WARNING_TAG("Material", "Resource '{}' doesn't exist", Name);
            return;
        }

        Entry->Resource = Resource;
        Entry->Desc     = Desc;
        std::visit(
            VariantVisitor{
                [](const auto&) {},
                [Entry, Descriptor](
                    const CBVDesc& Desc)
                {
                    CBVDesc FixedDesc{
                        Entry->Resource->GetHandle(Desc.Resource),
                        Desc.Size
                    };

                    Descriptor->Heap->CreateConstantBufferView(
                        Entry->Offset + Descriptor->Offset,
                        FixedDesc);
                },
                [Entry, Descriptor](
                    const SRVDescOpt& Desc)
                {
                    Descriptor->Heap->CreateShaderResourceView(
                        Entry->Offset + Descriptor->Offset,
                        Entry->Resource.get(),
                        Desc.has_value() ? &*Desc : nullptr);
                },
                [Entry, Descriptor](
                    const UAVDescOpt& Desc)
                {
                    Descriptor->Heap->CreateUnorderedAccessView(
                        Entry->Offset + Descriptor->Offset,
                        Entry->Resource.get(),
                        Desc.has_value() ? &*Desc : nullptr);
                },
            },
            Desc);
    }

    void Material::SetSampler(
        const StringU8&       Name,
        const SamplerDescOpt& Desc)
    {
        SamplerEntry*         Entry      = nullptr;
        DescriptorHeapHandle* Descriptor = nullptr;

        if (auto Iter = m_LocalParameters.Entries.find(Name);
            Iter != m_LocalParameters.Entries.end())
        {
            Entry      = std::get_if<SamplerEntry>(&Iter->second);
            Descriptor = &m_LocalParameters.Descriptors.SamplerDescriptors;
        }
        else if (auto Iter = m_SharedParameters->Entries.find(Name);
                 Iter != m_SharedParameters->Entries.end())
        {
            Entry      = std::get_if<SamplerEntry>(&Iter->second);
            Descriptor = &m_SharedParameters->Descriptors.SamplerDescriptors;
        }

        if (!Entry)
        {
            NEON_WARNING_TAG("Material", "Sampler '{}' doesn't exist", Name);
            return;
        }

        Entry->Desc = Desc;
        Descriptor->Heap->CreateSampler(
            Entry->Offset + Descriptor->Offset,
            Desc ? *Desc : SamplerDesc::Null);
    }

    void Material::SetData(
        const StringU8& Name,
        const void*     Data,
        size_t          ArrayOffset)
    {
        RootEntry* Entry = nullptr;

        Entry     = &m_LocalParameters.Buffer;
        auto View = Entry->Struct[Name];

        if (!View)
        {
            Entry = &m_SharedParameters->Buffer;
            View  = Entry->Struct[Name];
        }

        if (!View)
        {
            NEON_WARNING_TAG("Material", "Resource '{}' doesn't exist", Name);
            return;
        }

        Structured::BufferView Buffer(Entry->MappedData, View, ArrayOffset);
        std::copy(std::bit_cast<uint8_t*>(Data), std::bit_cast<uint8_t*>(Data) + View.GetSize(), Buffer.Data());
    }

    //

    bool Material::GetResource(
        const StringU8&     Name,
        Ptr<IGpuResource>*  Resource,
        DescriptorViewDesc* Desc)
    {
        ResourceEntry*        Entry      = nullptr;
        DescriptorHeapHandle* Descriptor = nullptr;

        if (auto Iter = m_LocalParameters.Entries.find(Name);
            Iter != m_LocalParameters.Entries.end())
        {
            Entry      = std::get_if<ResourceEntry>(&Iter->second);
            Descriptor = &m_LocalParameters.Descriptors.ResourceDescriptors;
        }
        else if (auto Iter = m_SharedParameters->Entries.find(Name);
                 Iter != m_SharedParameters->Entries.end())
        {
            Entry      = std::get_if<ResourceEntry>(&Iter->second);
            Descriptor = &m_SharedParameters->Descriptors.ResourceDescriptors;
        }

        if (!Entry)
        {
            return false;
        }

        if (Resource)
        {
            *Resource = Entry->Resource;
        }
        if (Desc)
        {
            *Desc = Entry->Desc;
        }
        return true;
    }

    bool Material::GetSampler(
        const StringU8& Name,
        SamplerDescOpt* Desc)
    {
        SamplerEntry*         Entry      = nullptr;
        DescriptorHeapHandle* Descriptor = nullptr;

        if (auto Iter = m_LocalParameters.Entries.find(Name);
            Iter != m_LocalParameters.Entries.end())
        {
            Entry      = std::get_if<SamplerEntry>(&Iter->second);
            Descriptor = &m_LocalParameters.Descriptors.SamplerDescriptors;
        }
        else if (auto Iter = m_SharedParameters->Entries.find(Name);
                 Iter != m_SharedParameters->Entries.end())
        {
            Entry      = std::get_if<SamplerEntry>(&Iter->second);
            Descriptor = &m_SharedParameters->Descriptors.SamplerDescriptors;
        }

        if (!Entry)
        {
            return false;
        }
        if (Desc)
        {
            *Desc = Entry->Desc;
        }
        return true;
    }

    size_t Material::GetData(
        const StringU8& Name,
        const void*     Data,
        size_t          ArrayOffset)
    {
        RootEntry* Entry = nullptr;

        Entry     = &m_LocalParameters.Buffer;
        auto View = Entry->Struct[Name];

        if (!View)
        {
            Entry = &m_SharedParameters->Buffer;
            View  = Entry->Struct[Name];
        }

        if (!View)
        {
            return 0;
        }

        if (Data)
        {
            Structured::BufferView Buffer(Entry->MappedData, View, ArrayOffset);
            std::copy(Buffer.Data(), Buffer.Data() + View.GetSize(), std::bit_cast<uint8_t*>(Data));
        }

        return View.GetSize();
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

    //

    Material::RootEntry::RootEntry(
        const RootEntry& Other) :
        Data(Other.Data.Size, ShaderResourceAlignement, Other.Data.Type),
        MappedData(Data ? Data.AsUpload().Map() + Data.Offset : nullptr),
        Struct(Other.Struct)
    {
    }
} // namespace Neon::RHI