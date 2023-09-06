#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <Runtime/DebugOverlay.hpp>

#include <RHI/Swapchain.hpp>
#include <RHI/Resource/State.hpp>

#include <Runtime/GameEngine.hpp>
#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Camera.hpp>

#include <Log/Logger.hpp>

#define NEON_RENDER_GRAPH_THREADED

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

    void RenderGraph::Update(
        const Scene::Component::Camera&    Camera,
        const Scene::Component::Transform& Transform)
    {
        auto& CameraBuffer = m_Storage.MapFrameData();

        CameraBuffer.World = glm::transpose(Transform.World.ToMat4x4());

        CameraBuffer.View           = glm::transpose(Camera.ViewMatrix());
        CameraBuffer.Projection     = glm::transpose(Camera.ProjectionMatrix());
        CameraBuffer.ViewProjection = CameraBuffer.View * CameraBuffer.Projection;

        CameraBuffer.ViewInverse           = glm::inverse(CameraBuffer.View);
        CameraBuffer.ProjectionInverse     = glm::inverse(CameraBuffer.Projection);
        CameraBuffer.ViewProjectionInverse = glm::inverse(CameraBuffer.ViewProjection);

        CameraBuffer.EngineTime = float(Runtime::GameEngine::Get()->GetEngineTime());
        CameraBuffer.GameTime   = float(Runtime::GameEngine::Get()->GetGameTime());
        CameraBuffer.DeltaTime  = float(Runtime::GameEngine::Get()->GetDeltaTime());

        m_Storage.UnmapFrameData();
    }

    void RenderGraph::Draw(
        bool CopyToBackBuffer)
    {
        m_Storage.NewOutputImage();

        for (auto& ImportedResource : m_Storage.m_ImportedResources)
        {
            auto& Handle = m_Storage.GetResourceMut(ImportedResource);
            m_Storage.CreateViews(Handle);
        }

        // We will cache the command contexts to avoid submitting single command list per pass + barrier flush
        {
            ChainedCommandList ChainedCommandList;

            for (auto& Level : m_Levels)
            {
                Level.Execute(ChainedCommandList);
            }
        }

        m_Storage.FlushResources();
    }

    void RenderGraph::Build(
        std::vector<GraphDepdencyLevel>&& Levels)
    {
        m_Levels = std::move(Levels);
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
        RenderGraph::ChainedCommandList& ChainedCommandList) const
    {
        auto& Storage = m_Context.GetStorage();

        for (auto& Id : m_ResourcesToCreate)
        {
            auto& Handle = Storage.GetResourceMut(Id);
            Storage.ReallocateResource(Handle);
            Storage.CreateViews(Handle);
        }

        ExecuteBarriers(ChainedCommandList);
        ExecutePasses(ChainedCommandList);

        for (auto& Id : m_ResourcesToDestroy)
        {
            auto& Handle = Storage.GetResource(Id);
            if (!Handle.IsImported())
            {
                Storage.FreeResource(Handle);
            }
        }
    }

    void GraphDepdencyLevel::ExecuteBarriers(
        RenderGraph::ChainedCommandList& ChainedCommandList) const
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

        StateManager->FlushBarriers(ChainedCommandList.Load());
    }

    //

    void GraphDepdencyLevel::ExecutePasses(
        RenderGraph::ChainedCommandList& ChainedCommandList) const
    {
        // If we have more than one pass, we need to synchronize them
        // therefore we need to flush the chained command list we previously created
        const bool IsDirect    = m_Passes[0].Pass->GetQueueType() == PassQueueType::Direct;
        const bool ShouldFlush = m_Passes.size() > 1 || (ChainedCommandList.CommandList && IsDirect != ChainedCommandList.IsDirect);
        if (ShouldFlush)
        {
            ChainedCommandList.Flush();
        }

        auto DispatchTask =
            [this, &ChainedCommandList, ShouldFlush](
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
                if (Threaded && ShouldFlush)
                {
                    std::scoped_lock Lock(m_Context.m_RenderMutex);
                    RenderCommandList = ChainedCommandList.RenderContext.Append();
                }
                else
                {
                    RenderCommandList = dynamic_cast<RHI::IGraphicsCommandList*>(ChainedCommandList.Load());
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
                    if (ClearValue)
                    {
                        Depth   = ClearValue->Depth;
                        Stencil = ClearValue->Stencil;
                    }

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
                            if (ViewDesc->ForceDepth)
                            {
                                Depth = *ViewDesc->ForceDepth;
                            }
                            Stencil.reset();

                            break;
                        }
                        case RHI::EDSClearType::Stencil:
                        {
                            if (ViewDesc->ForceStencil)
                            {
                                Stencil = *ViewDesc->ForceStencil;
                            }
                            Depth.reset();

                            break;
                        }
                        case RHI::EDSClearType::DepthStencil:
                        {
                            if (ViewDesc->ForceDepth)
                            {
                                Depth = *ViewDesc->ForceDepth;
                            }
                            if (ViewDesc->ForceStencil)
                            {
                                Stencil = *ViewDesc->ForceStencil;
                            }

                            break;
                        }

                        default:
                            std::unreachable();
                        }
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
                if (Threaded && ShouldFlush)
                {
                    std::scoped_lock Lock(m_Context.m_RenderMutex);
                    CommandList = ChainedCommandList.RenderContext.Append();
                }
                else
                {
                    CommandList = ChainedCommandList.Load();
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

            ChainedCommandList.FlushOrDelay();
        }
        // Here we can just execute in the current thread rather than spawning a new one
        else
        {
            if (m_Passes[0].Pass->GetQueueType() != PassQueueType::Unknown &&
                !m_Passes[0].Pass->GetPassFlags().Test(EPassFlags::Cull))
            {
                ChainedCommandList.Preload(IsDirect);
                DispatchTask(0, false);
            }
        }
    }

    RHI::ICommonCommandList* RenderGraph::ChainedCommandList::Load()
    {
        if (CommandList) [[likely]]
        {
            return CommandList;
        }

        IsDirect           = true;
        return CommandList = RenderContext.Append();
    }

    void RenderGraph::ChainedCommandList::Preload(
        bool IsDirect)
    {
        if (CommandList)
        {
            if (this->IsDirect == IsDirect)
            {
                return;
            }
            Flush();
        }
        if (IsDirect)
        {
            CommandList = RenderContext.Append();
        }
        else
        {
            CommandList = ComputeContext.Append();
        }
        this->IsDirect = IsDirect;
    }

    void RenderGraph::ChainedCommandList::Flush()
    {
        CommandList = nullptr;
        RenderContext.Upload();
        ComputeContext.Upload();
    }

    void RenderGraph::ChainedCommandList::FlushOrDelay()
    {
        if (RenderContext.Size() == 1)
        {
            IsDirect    = true;
            CommandList = RenderContext[0];
        }
        else if (ComputeContext.Size() == 1)
        {
            IsDirect    = false;
            CommandList = ComputeContext[0];
        }
        else
        {
            Flush();
        }
    }
} // namespace Neon::RG
