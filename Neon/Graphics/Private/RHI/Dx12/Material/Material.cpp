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
                    Source.GetCpuHandle(Source.Offset),
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

        for (auto& [Name, ParamIndex] : RootSignature->GetNamedParams())
        {
            auto& Param = RootSignature->GetParams()[ParamIndex];
            boost::apply_visitor(
                VariantVisitor{
                    [&BufferSize](const IRootSignature::ParamConstant& Constant)
                    {
                        BufferSize += Constant.Num32BitValues * sizeof(uint32_t);
                    },
                    [Mat, &Name](const IRootSignature::ParamRoot& Root)
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
                        Mat->m_SharedParameters->SharedEntries.emplace(Name, Material::RootEntry{ .ViewType = ViewType });
                    },
                    [&](const IRootSignature::ParamDescriptor& Descriptor)
                    {
                        const bool IsSamplerDescriptor = Descriptor.NamedRanges.begin()->second.Type == DescriptorTableParam::Sampler;
                        if (IsSamplerDescriptor)
                        {
                            auto& DescriptorCount = Descriptor.Instanced ? TableSamplerCount : TableSharedSamplerCount;
                            DescriptorCount += Descriptor.Size;

                            for (auto& [Name, Range] : Descriptor.NamedRanges)
                            {
                                Material::SamplerEntry Entry{
                                    .Offset = DescriptorCount,
                                    .Count  = Range.Size
                                };

                                Entry.Descs.resize(Range.Size);

                                if (Descriptor.Instanced)
                                {
                                    Mat->m_SharedParameters->SharedEntries.emplace(Name, std::move(Entry));
                                }
                                else
                                {
                                    Mat->m_LocalParameters.LocalEntries.emplace(Name, std::move(Entry));
                                }
                            }
                        }
                        else
                        {
                            auto& DescriptorCount = Descriptor.Instanced ? TableResourceCount : TableSharedResourceCount;
                            DescriptorCount += Descriptor.Size;

                            for (auto& [Name, Range] : Descriptor.NamedRanges)
                            {
                                Material::DescriptorEntry Entry{
                                    .Offset = DescriptorCount,
                                    .Count  = Range.Size,
                                    .Type   = Range.Type
                                };

                                Entry.Resources.resize(Range.Size);

                                if (Descriptor.Instanced)
                                {
                                    Mat->m_SharedParameters->SharedEntries.emplace(Name, std::move(Entry));
                                }
                                else
                                {
                                    Mat->m_LocalParameters.LocalEntries.emplace(Name, std::move(Entry));
                                }
                            }
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

            for (size_t i = 0; i < size_t(IMaterial::PipelineVariant::Count); i++)
            {
                switch (IMaterial::PipelineVariant(i))
                {
                case IMaterial::PipelineVariant::RenderPass:
                {
                    PipelineDesc.PixelShader = Builder.PixelShader();
                    break;
                }
                case IMaterial::PipelineVariant::DepthPass:
                {
                    PipelineDesc.PixelShader = nullptr;
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
        const UnqiueDescriptorHeapHandle& Other)
    {
        CreateAndInitializeDescriptorIfNeeded(ResourceDescriptors, DescriptorType::ResourceView, Other.ResourceDescriptors);
        CreateAndInitializeDescriptorIfNeeded(SamplerDescriptors, DescriptorType::Sampler, Other.SamplerDescriptors);
    }

    auto Material::UnqiueDescriptorHeapHandle::operator=(
        const UnqiueDescriptorHeapHandle& Other) -> UnqiueDescriptorHeapHandle&
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
        UnqiueDescriptorHeapHandle&& Other) -> UnqiueDescriptorHeapHandle&
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