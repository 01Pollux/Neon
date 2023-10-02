#include <EnginePCH.hpp>

#include <Math/Frustum.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/LightCullPass.hpp>

#include <RHI/Shaders/GridFrustumGen.hpp>
#include <RHI/Shaders/LightCull.hpp>
#include <RHI/PipelineState.hpp>
#include <RHI/Resource/State.hpp>

namespace Neon::RG
{
    LightCullPass::LightCullPass() :
        RenderPass("LightCullPass")
    {
        RHI::Shaders::GridFrustumGenShader GridFrustumGenShader;
        RHI::Shaders::LightCullShader      LightCullShader;

        m_GridFrustumRS =
            RHI::RootSignatureBuilder(STR("GridFrustumGen::RootSignature"))
                .Add32BitConstants<Size2I>("c_DispatchConstants", 0, 1)
                .AddConstantBufferView("_FrameConstant", 0, 0, RHI::ShaderVisibility::All)
                .AddDescriptorTable(
                    "c_Frustum",
                    RHI::RootDescriptorTable()
                        .AddUavRange("", 0, 1, 1),
                    RHI::ShaderVisibility::All)
                .ComputeOnly()
                .AddStandardSamplers()
                .Build();

        uint32_t DescriptorOffset = 0;
        m_LightCullRS =
            RHI::RootSignatureBuilder(STR("LightCull::RootSignature"))
                .Add32BitConstants<uint32_t>("c_LightInfo", 0, 1)
                .AddConstantBufferView("_FrameConstant", 0, 0, RHI::ShaderVisibility::All)
                .AddDescriptorTable(
                    "c_TextureTable",
                    RHI::RootDescriptorTable()
                        .AddSrvRangeAt("c_DepthBuffer", 0, 1, 1, DescriptorOffset++)
                        .AddSrvRangeAt("c_FrustumGrid", 1, 1, 1, DescriptorOffset++)
                        .AddSrvRangeAt("c_Lights", 2, 1, 1, DescriptorOffset++)
                        .AddUavRangeAt("c_LightResult", 0, 1, 1, DescriptorOffset++)
                        .AddUavRangeAt("c_LightIndexList_Opaque", 1, 1, 1, DescriptorOffset++)
                        .AddUavRangeAt("c_LightIndexList_Transparent", 2, 1, 1, DescriptorOffset++)
                        .AddUavRangeAt("c_LightGrid_Opaque", 3, 1, 1, DescriptorOffset++)
                        .AddUavRangeAt("c_LightGrid_Transparent", 4, 1, 1, DescriptorOffset++),
                    RHI::ShaderVisibility::All)
                .ComputeOnly()
                .AddStandardSamplers()
                .Build();

        m_GridFrustumPSO =
            RHI::PipelineStateBuilderC{
                .RootSignature = m_GridFrustumRS,
                .ComputeShader = GridFrustumGenShader->LoadShader({ .Stage = RHI::ShaderStage::Compute })
            }
                .Build();

        m_LightCullPSO =
            RHI::PipelineStateBuilderC{
                .RootSignature = m_LightCullRS,
                .ComputeShader = LightCullShader->LoadShader({ .Stage = RHI::ShaderStage::Compute })
            }
                .Build();
    }

    LightCullPass::~LightCullPass()
    {
        if (m_GridFrustumViews)
        {
            RHI::IStaticDescriptorHeap::Get(RHI::DescriptorType::ResourceView)->Free(m_GridFrustumViews);
        }
    }

    void LightCullPass::ResolveResources(
        ResourceResolver& Resolver)
    {
    }

    void LightCullPass::DispatchTyped(
        const GraphStorage&     Storage,
        RHI::ComputeCommandList CommandList)
    {
        RecreateGridFrustumIfNeeded(Storage, CommandList);
    }

    //

    void LightCullPass::RecreateGridFrustumIfNeeded(
        const GraphStorage&     Storage,
        RHI::ComputeCommandList CommandList)
    {
        auto OutputSize = Storage.GetOutputImageSize();
        if (m_GridSize == OutputSize) [[likely]]
        {
            return;
        }

        m_GridSize      = OutputSize;
        auto& GroupSize = m_GridFrustumPSO->GetComputeGroupSize();

        Size2I GridCount(
            int(Math::DivideByMultiple(OutputSize.x, GroupSize.x)),
            int(Math::DivideByMultiple(OutputSize.y, GroupSize.y)));

        uint32_t BufferCount = GridCount.x * GridCount.y;
        size_t   SizeInBytes = sizeof(Geometry::Frustum) * BufferCount;

        RHI::MResourceFlags Flags;
        Flags.Set(RHI::EResourceFlags::AllowUnorderedAccess);

        auto Descriptor = RHI::IStaticDescriptorHeap::Get(RHI::DescriptorType::ResourceView);

        if (m_GridFrustumViews)
        {
            Descriptor->Free(m_GridFrustumViews);
        }

        m_GridFrustum.reset(RHI::IGpuResource::Create(
            RHI::ResourceDesc::Buffer(
                SizeInBytes,
                Flags)));

        m_GridFrustumViews = Descriptor->Allocate(2);

        RHI::UAVDesc UavDesc{
            .View = RHI::UAVDesc::Buffer{
                .Count        = BufferCount,
                .SizeOfStruct = sizeof(Geometry::Frustum) }
        };
        m_GridFrustumViews->CreateUnorderedAccessView(
            m_GridFrustumViews.Offset,
            m_GridFrustum.get(),
            &UavDesc);

        RHI::SRVDesc SrvDesc{
            .View = RHI::SRVDesc::Buffer{
                .Count        = BufferCount,
                .SizeOfStruct = sizeof(Geometry::Frustum) }
        };
        m_GridFrustumViews->CreateShaderResourceView(
            m_GridFrustumViews.Offset + 1,
            m_GridFrustum.get(),
            &SrvDesc);

        DispatchGridFrustum(Storage, CommandList, GridCount);
    }

    void LightCullPass::DispatchGridFrustum(
        const GraphStorage&     Storage,
        RHI::ComputeCommandList CommandList,
        const Size2I&           GridCount)
    {
        auto StateManager = RHI::IResourceStateManager::Get();

        //

        CommandList.SetPipelineState(m_GridFrustumPSO);
        CommandList.SetRootSignature(m_GridFrustumRS);

        CommandList.SetConstants(
            0,
            &GridCount.x,
            Math::DivideByMultiple(sizeof(GridCount), sizeof(uint32_t)));

        CommandList.SetResourceView(
            RHI::CstResourceViewType::Cbv,
            1,
            Storage.GetFrameDataHandle());

        CommandList.SetDynamicDescriptorTable(
            2,
            m_GridFrustumViews.GetCpuHandle(),
            1,
            false);

        StateManager->TransitionResource(m_GridFrustum.get(), RHI::EResourceState::UnorderedAccess);
        StateManager->FlushBarriers(CommandList);

        CommandList.Dispatch(
            m_GridSize.x,
            m_GridSize.y);

        StateManager->TransitionResource(m_GridFrustum.get(), RHI::EResourceState::NonPixelShaderResource);
        StateManager->FlushBarriers(CommandList);
    }
} // namespace Neon::RG