#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <Runtime/DebugOverlay.hpp>

#include <RHI/Swapchain.hpp>
#include <RHI/Resource/State.hpp>

#include <Log/Logger.hpp>

// #define NEON_RENDER_GRAPH_THREADED

namespace Neon::RG
{
    GraphBuilder RenderGraph::Reset()
    {
        m_Storage.Reset();
        return GraphBuilder(*this);
    }

    GraphStorage& RenderGraph::GetStorage() noexcept
    {
        return m_Storage;
    }

    const GraphStorage& RenderGraph::GetStorage() const noexcept
    {
        return m_Storage;
    }

    void RenderGraph::Run(
        RHI::GpuResourceHandle CameraBuffer,
        bool                   CopyToBackBuffer)
    {
        for (auto& ImportedResource : m_Storage.m_ImportedResources)
        {
            auto& Handle = m_Storage.GetResourceMut(ImportedResource);
            m_Storage.CreateViews(Handle);
        }
        {

            RenderGraph::RenderCommandContext  RenderContext;
            RenderGraph::ComputeCommandContext ComputeContext;

            for (auto& Level : m_Levels)
            {
                Level.Execute(RenderContext, ComputeContext);
            }

            if (CopyToBackBuffer)
            {
                SubmitToBackBuffer(RenderContext.Append(), CameraBuffer);
            }
        }
        m_Storage.FlushResources();
    }

    void RenderGraph::Build(
        std::vector<GraphDepdencyLevel>&& Levels)
    {
        m_Levels = std::move(Levels);
    }

    void RenderGraph::SubmitToBackBuffer(
        RHI::ICommonCommandList* CommandList,
        RHI::GpuResourceHandle   CameraBuffer)
    {
        auto OutputImage = m_Storage.GetResource(RG::ResourceId(STR("OutputImage"))).AsTexture();

        auto Backbuffer   = RHI::ISwapchain::Get()->GetBackBuffer();
        auto StateManager = RHI::IResourceStateManager::Get();

        //

        // Transition the backbuffer to a copy destination.
        StateManager->TransitionResource(
            Backbuffer,
            RHI::MResourceState::FromEnum(RHI::EResourceState::CopyDest));

        // Transition the output image to a copy source.
        StateManager->TransitionResource(
            OutputImage.get(),
            RHI::MResourceState::FromEnum(RHI::EResourceState::CopySource));

        // Prepare the command list.
        StateManager->FlushBarriers(CommandList);
        CommandList->CopyResource(Backbuffer, OutputImage.get());

        // Transition the backbuffer to a present state.
        StateManager->TransitionResource(
            Backbuffer,
            RHI::MResourceState_Present);

        StateManager->FlushBarriers(CommandList);
    }

    //

    GraphDepdencyLevel::GraphDepdencyLevel(
        RenderGraph& Context) :
        m_Context(Context)
    {
    }

    //

    void GraphDepdencyLevel::AddPass(
        UPtr<IRenderPass>                             Pass,
        std::vector<ResourceViewId>                   RenderTargets,
        std::optional<ResourceViewId>                 DepthStencil,
        std::set<ResourceId>                          ResourceToCreate,
        std::set<ResourceId>                          ResourceToDestroy,
        std::map<ResourceViewId, RHI::MResourceState> States)
    {
        m_Passes.emplace_back(std::move(Pass), std::move(RenderTargets), std::move(DepthStencil));
        m_ResourcesToCreate.merge(std::move(ResourceToCreate));
        m_ResourcesToDestroy.merge(std::move(ResourceToDestroy));
        for (auto& [ViewId, State] : States)
        {
            auto& CurrentState = m_States[ViewId];
            CurrentState |= State;
        }
    }

    void GraphDepdencyLevel::Execute(
        RenderGraph::RenderCommandContext&  RenderContext,
        RenderGraph::ComputeCommandContext& ComputeContext) const
    {
        auto& Storage = m_Context.GetStorage();

        for (auto& Id : m_ResourcesToCreate)
        {
            auto& Handle = Storage.GetResourceMut(Id);
            Storage.RellocateResource(Handle);
            Storage.CreateViews(Handle);
        }

        ExecuteBarriers(RenderContext, ComputeContext);
        ExecutePasses(RenderContext, ComputeContext);

        for (auto& Id : m_ResourcesToDestroy)
        {
            auto& Handle = Storage.GetResource(Id);
            Storage.FreeResource(Handle);
        }
    }

    void GraphDepdencyLevel::ExecuteBarriers(
        RenderGraph::RenderCommandContext&  RenderContext,
        RenderGraph::ComputeCommandContext& ComputeContext) const
    {
        auto& Storage      = m_Context.GetStorage();
        auto  StateManager = RHI::IResourceStateManager::Get();

        for (auto& [ViewId, State] : m_States)
        {
            auto& Handle = Storage.GetResource(ViewId.GetResource());
            StateManager->TransitionResource(
                Handle.Get().get(),
                State,
                ViewId.GetSubresourceIndex());
        }

        StateManager->FlushBarriers();
    }

    //

    void GraphDepdencyLevel::ExecutePasses(
        RenderGraph::RenderCommandContext&  RenderContext,
        RenderGraph::ComputeCommandContext& ComputeContext) const
    {
        auto DispatchTask =
            [this, &RenderContext, &ComputeContext](
                size_t PassIndex,
                bool   Threaded)
        {
            auto& [RenderPass, RenderTargets, DepthStencil] = m_Passes[PassIndex];
            auto& Storage                                   = m_Context.GetStorage();

            RHI::ICommandList* CommandList = nullptr;

            switch (RenderPass->GetQueueType())
            {
            case PassQueueType::Direct:
            {
                RHI::IGraphicsCommandList* RenderCommandList;
                {
                    std::scoped_lock Lock(m_Context.m_RenderMutex);
                    RenderCommandList = RenderContext.Append();
                }
                CommandList = RenderCommandList;

                std::vector<RHI::CpuDescriptorHandle> RtvHandles;
                RtvHandles.reserve(RenderTargets.size());

                RHI::CpuDescriptorHandle  DsvHandle;
                RHI::CpuDescriptorHandle* DsvHandlePtr = nullptr;

                for (auto& RtvViewId : RenderTargets)
                {
                    RHI::CpuDescriptorHandle RtvHandle;

                    auto& Handle   = Storage.GetResource(RtvViewId.GetResource());
                    auto& ViewDesc = std::get<std::optional<RHI::RTVDesc>>(Storage.GetResourceView(RtvViewId, &RtvHandle));

                    auto& Desc = Handle.GetDesc();
                    RtvHandles.emplace_back(RtvHandle);

                    if (ViewDesc && ViewDesc->ClearType != RHI::ERTClearType::Ignore)
                    {
#if NEON_DEBUG
                        if (!ViewDesc->ForceColor && !Desc.ClearValue)
                        {
                            NEON_WARNING_TAG("RenderGraph", "Render target view has no clear value, while clear type is not set to Ignore");
                        }
                        else
#endif
                        {
                            RenderCommandList->ClearRtv(
                                RtvHandle,
                                ViewDesc->ForceColor.value_or(
                                    std::get<Color4>(Desc.ClearValue->Value)));
                        }
                    }
                }

                if (DepthStencil)
                {
                    auto& Handle   = Storage.GetResource(DepthStencil->GetResource());
                    auto& ViewDesc = std::get<std::optional<RHI::DSVDesc>>(Storage.GetResourceView(*DepthStencil, &DsvHandle));
                    auto& Desc     = Handle.GetDesc();

                    std::optional<float>   Depth;
                    std::optional<uint8_t> Stencil;

                    auto ClearValue = Desc.ClearValue ? std::get_if<RHI::ClearOperation::DepthStencil>(&Desc.ClearValue->Value) : nullptr;

                    if (ViewDesc)
                    {
#if NEON_DEBUG
                        if (!ViewDesc->ForceDepth && !ClearValue)
                        {
                            NEON_WARNING_TAG("RenderGraph", "Depth stencil view has no clear value, while clear type is not set to Ignore");
                        }
#endif

                        switch (ViewDesc->ClearType)
                        {
                        case RHI::EDSClearType::Depth:
                        {
                            Depth = ViewDesc->ForceDepth ? *ViewDesc->ForceDepth : ClearValue->Depth;
                            break;
                        }
                        case RHI::EDSClearType::Stencil:
                        {
                            Stencil = ViewDesc->ForceStencil ? *ViewDesc->ForceStencil : ClearValue->Stencil;
                            break;
                        }
                        case RHI::EDSClearType::DepthStencil:
                        {
                            Depth   = ViewDesc->ForceDepth ? *ViewDesc->ForceDepth : ClearValue->Depth;
                            Stencil = ViewDesc->ForceStencil ? *ViewDesc->ForceStencil : ClearValue->Stencil;
                            break;
                        }

                        default:
                            std::unreachable();
                        }
                    }
                    else if (ClearValue)
                    {
                        Depth   = ClearValue->Depth;
                        Stencil = ClearValue->Stencil;
                    }
#if NEON_DEBUG
                    else
                    {
                        NEON_WARNING_TAG("RenderGraph", "Depth stencil view has no clear value, while clear type is not set to Ignore");
                    }
#endif

                    RenderCommandList->ClearDsv(DsvHandle, Depth, Stencil);
                    DsvHandlePtr = &DsvHandle;
                }

                RenderCommandList->SetRenderTargets(
                    RtvHandles.data(),
                    RtvHandles.size(),
                    DsvHandlePtr);

                if (!RenderPass->OverrideViewport(Storage, RenderCommandList))
                {
                    auto& Size = RHI::ISwapchain::Get()->GetSize();

                    RenderCommandList->SetViewport(
                        ViewportF{
                            .Width    = float(Size.Width()),
                            .Height   = float(Size.Height()),
                            .MaxDepth = 1.f,
                        });
                    RenderCommandList->SetScissorRect(RectF(Vec::Zero<Vector2>, Size));
                }

                break;
            }

            case PassQueueType::Compute:
            {
                {
                    std::scoped_lock Lock(m_Context.m_RenderMutex);
                    CommandList = RenderContext.Append();
                }
                break;
            }
            }

#ifndef NEON_DIST
            RHI::RenameObject(CommandList, RenderPass->GetPassName());
#endif
            RenderPass->Dispatch(Storage, CommandList);
        };

        if (m_Passes.size())
        {
#ifdef NEON_RENDER_GRAPH_THREADED
            std::vector<std::future<void>> Futures;
            Futures.reserve(m_Passes.size());
#endif

            for (size_t i = 0; i < m_Passes.size(); i++)
            {
                if (m_Passes[i].Pass->GetQueueType() == PassQueueType::Unknown ||
                    m_Passes[i].Pass->GetPassFlags().Test(EPassFlags::Cull))
                {
                    continue;
                }

#ifdef NEON_RENDER_GRAPH_THREADED
                Futures.emplace_back(m_Context.m_ThreadPool.enqueue(DispatchTask, i, true));
#else
                DispatchTask(i, true);
#endif
            }

#ifdef NEON_RENDER_GRAPH_THREADED
            for (auto& Future : Futures)
            {
                Future.get();
            }
#endif
        }
        // Here we can just execute in the current thread rather than spawning a new one
        else
        {
            if (m_Passes[0].Pass->GetQueueType() != PassQueueType::Unknown &&
                !m_Passes[0].Pass->GetPassFlags().Test(EPassFlags::Cull))
            {
                DispatchTask(0, false);
            }
        }

        RenderContext.Upload();
        ComputeContext.Upload();
    }
} // namespace Neon::RG
