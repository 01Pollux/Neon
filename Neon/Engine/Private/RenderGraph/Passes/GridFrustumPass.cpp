#include <EnginePCH.hpp>

#include <Math/Frustum.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/GridFrustumPass.hpp>

#include <RHI/PipelineState.hpp>
#include <RHI/Resource/State.hpp>

namespace Neon::RG
{
    GridFrustumPass::GridFrustumPass() :
        RenderPass("GridFrustumPass")
    {
    }

    void GridFrustumPass::ResolveResources(
        ResourceResolver& Resolver)
    {
        /* auto Desc = RHI::ResourceDesc::Buffer(
             Resolver.GetSwapchainFormat(),
             0, 0, 1, 1);

         const ResourceId GridFrustum("GridFrustum");

         Resolver.CreateWindowTexture(
             GridFrustum,
             Desc);*/
    }

    void GridFrustumPass::DispatchTyped(
        const GraphStorage&     Storage,
        RHI::ComputeCommandList CommandList)
    {
        RecreateGridFrustum(Storage, CommandList);
    }

    void GridFrustumPass::RecreateGridFrustum(
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

        size_t SizeInBytes = sizeof(Geometry::Frustum) * GridCount.x * GridCount.y;

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

        m_GridFrustumViews->CreateUnorderedAccessView(
            m_GridFrustumViews.Offset,
            m_GridFrustum.get());

        m_GridFrustumViews->CreateShaderResourceView(
            m_GridFrustumViews.Offset + 1,
            m_GridFrustum.get());

        DispatchGridFrustum(Storage, CommandList, GridCount);
    }

    void GridFrustumPass::DispatchGridFrustum(
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