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
        const wchar_t*                       Name,
        const GenericMaterialBuilder<false>& Builder)
    {
        return Ptr<Material>{ NEON_NEW Material(Name, Builder) };
    }

    Ptr<IMaterial> IMaterial::Create(
        const wchar_t*                      Name,
        const GenericMaterialBuilder<true>& Builder)
    {
        return Ptr<Material>{ NEON_NEW Material(Name, Builder) };
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

    //

    template<bool _Compute>
    void Material_CreateDescriptors(
        const wchar_t*                          Name,
        const GenericMaterialBuilder<_Compute>& Builder,
        Material*                               Mat)
    {
        Mat->m_RootSignature = Dx12RootSignatureCache::Compile(Name, Builder.Shaders());
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

            // Mat->m_PipelineState = IPipelineState::Create(PipelineDesc);
        }
        else
        {
            PipelineStateBuilderC PipelineDesc{
                .RootSignature = Mat->m_RootSignature,
                .ComputeShader = Builder.ComputeShader()
            };

            // Mat->m_PipelineState = IPipelineState::Create(PipelineDesc);
        }
    }

    //

    Material::Material(
        const wchar_t*               Name,
        const RenderMaterialBuilder& Builder)
    {
        Material_CreateDescriptors(
            Name,
            Builder,
            this);

        Material_CreatePipelineState(
            Builder,
            this);
    }

    Material::Material(
        const wchar_t*                Name,
        const ComputeMaterialBuilder& Builder)
    {
        Material_CreateDescriptors(
            Name,
            Builder,
            this);

        // TODO: Rework material pipeline state creation
        // Material_CreatePipelineState(
        //    Builder,
        //    this);
    }

    Material::Material(
        Material* Other) :
        m_LocalDescriptors(Other->m_LocalDescriptors.ResourceDescriptors.Size, Other->m_LocalDescriptors.SamplerDescriptors.Size)
    {
        m_RootSignature  = Other->m_RootSignature;
        m_PipelineStates = Other->m_PipelineStates;
    }

    Ptr<IMaterial> Material::CreateInstance()
    {
        return Ptr<IMaterial>(NEON_NEW Material(this));
    }

    //

    //

    Material::UnqiueDescriptorHeapHandle::UnqiueDescriptorHeapHandle(
        uint32_t ResourceDescriptorSize,
        uint32_t SamplerDescriptorSize)
    {
        CreateDescriptorIfNeeded(ResourceDescriptors, DescriptorType::ResourceView, ResourceDescriptorSize);
        CreateDescriptorIfNeeded(SamplerDescriptors, DescriptorType::Sampler, SamplerDescriptorSize);
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
                auto Allocator = IStaticDescriptorHeap::Get(DescriptorType::ResourceView);
                Allocator->Free(ResourceDescriptors);
            }
            if (SamplerDescriptors)
            {
                auto Allocator = IStaticDescriptorHeap::Get(DescriptorType::Sampler);
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