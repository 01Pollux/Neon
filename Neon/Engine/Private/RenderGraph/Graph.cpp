#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <Runtime/DebugOverlay.hpp>

#include <RHI/Swapchain.hpp>
#include <RHI/Resource/State.hpp>
#include <RHI/Commands/Queue.hpp>
#include <RHI/Fence.hpp>

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

    void RenderGraph::Reset(
        RenderGraph* Graph)
    {
    }

    //

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
        auto& CameraBuffer = m_Storage.GetFrameData();

        auto View = glm::lookAt(
            Transform.World.GetPosition(),
            Transform.World.GetPosition() + Transform.World.GetLookDir(),
            Transform.World.GetUpDir());

        CameraBuffer.World = Transform.World.ToMat4x4Transposed();

        CameraBuffer.View           = glm::transpose(View);
        CameraBuffer.Projection     = glm::transpose(Camera.ProjectionMatrix());
        CameraBuffer.ViewProjection = CameraBuffer.View * CameraBuffer.Projection;

        CameraBuffer.ViewInverse           = glm::inverse(CameraBuffer.View);
        CameraBuffer.ProjectionInverse     = glm::inverse(CameraBuffer.Projection);
        CameraBuffer.ViewProjectionInverse = glm::inverse(CameraBuffer.ViewProjection);

        CameraBuffer.ScreenResolution = { Camera.Viewport.Width, Camera.Viewport.Height };

        CameraBuffer.EngineTime = float(Runtime::GameEngine::Get()->GetEngineTime());
        CameraBuffer.GameTime   = float(Runtime::GameEngine::Get()->GetGameTime());
        CameraBuffer.DeltaTime  = float(Runtime::GameEngine::Get()->GetDeltaTime());

        m_Storage.UpdateOutputImage(CameraBuffer.ScreenResolution);

        m_Storage.GetSceneContext().Update(
            Camera,
            Transform);
    }

    void RenderGraph::Dispatch()
    {
        m_CommandListContext.Begin();

        m_Storage.PrepareDispatch();
        for (auto& Level : m_Levels)
        {
            Level.Execute();
        }

        m_CommandListContext.End();
    }

    void RenderGraph::Build(
        std::vector<GraphDepdencyLevel>&& Levels)
    {
        m_Levels = std::move(Levels);

        uint32_t MaxGraphics = 0, MaxCompute = 0;
        uint32_t LastFlushedGraphics = 0, LastFlushedCompute = 0;

        for (size_t i = 0; i < m_Levels.size(); i++)
        {
            auto ThisLevel = &m_Levels[i];
            auto NextLevel = (i + 1 < m_Levels.size()) ? &m_Levels[i + 1] : nullptr;
            auto PrevLevel = (i > 0) ? &m_Levels[i - 1] : nullptr;

            auto [GraphicsCount, ComputeCount] = ThisLevel->GetCommandListCount();

            ThisLevel->m_FlushBarriers =
                ThisLevel->m_ResetCommands = (GraphicsCount + ComputeCount) > 1;
            ThisLevel->m_FlushCommands     = !NextLevel;

            // Try to mutate from compute to graphics to avoid a flush
            if (!ThisLevel->m_ResetCommands)
            {
                if (ComputeCount)
                {
                    ComputeCount  = 0;
                    GraphicsCount = 1;
                    ThisLevel->m_Passes[0].Pass->SetQueueType(PassQueueType::Direct);
                }
                ThisLevel->m_FlushBarriers = false;
            }

            if (PrevLevel)
            {
                PrevLevel->m_FlushCommands |= (GraphicsCount != PrevLevel->m_GraphicsCommandsToFlush) ||
                                              (ComputeCount != PrevLevel->m_ComputeCommandsToFlush) ||
                                              ThisLevel->m_FlushBarriers;

                bool FlushedCommands       = PrevLevel->m_FlushCommands;
                ThisLevel->m_ResetBarriers = FlushedCommands;
            }

            MaxGraphics = std::max(MaxGraphics, GraphicsCount);
            MaxCompute  = std::max(MaxCompute, ComputeCount);

            ThisLevel->m_GraphicsCommandsToFlush = GraphicsCount;
            ThisLevel->m_ComputeCommandsToFlush  = ComputeCount;

            if (!PrevLevel)
            {
                ThisLevel->m_GraphicsCommandsToReserve = GraphicsCount;
                ThisLevel->m_ComputeCommandsToReserve  = 0;

                if (ThisLevel->m_FlushBarriers && !ThisLevel->m_ResetCommands)
                {
                    ThisLevel->m_GraphicsCommandsToReserve = ThisLevel->m_GraphicsCommandsToReserve ? (ThisLevel->m_GraphicsCommandsToReserve - 1) : 0;
                }
            }
            else
            {
                ThisLevel->m_GraphicsCommandsToReserve = std::min(LastFlushedGraphics, GraphicsCount);
                ThisLevel->m_ComputeCommandsToReserve  = std::min(LastFlushedCompute, ComputeCount);
            }

            LastFlushedGraphics = ThisLevel->m_GraphicsCommandsToFlush;
            LastFlushedCompute  = ThisLevel->m_ComputeCommandsToFlush;
        }

        m_CommandListContext = CommandListContext(MaxGraphics, MaxCompute);
    }

    //

    /// <summary>
    /// Allocates a vector of command lists of type _Ty
    /// </summary>
    static void AllocateCommandLists(
        std::vector<RHI::ICommandList*>& List,
        std::vector<bool>&               ActiveList)
    {
        if (!List.empty())
        {
            auto Queue = RHI::ISwapchain::Get()->GetQueue(true);
            List       = Queue->AllocateCommandLists(RHI::CommandQueueType::Graphics, List.size());
            ActiveList.insert(ActiveList.end(), List.size(), true);
        }
    }

    /// <summary>
    /// Flushes a vector of command lists of type _Ty
    /// </summary>
    static void FreeCommandLists(
        std::vector<RHI::ICommandList*>& List,
        std::vector<bool>&               ActiveList)
    {
        if (!List.empty())
        {
            auto Queue = RHI::ISwapchain::Get()->GetQueue(true);
            Queue->FreeCommandLists(RHI::CommandQueueType::Graphics, List);
            ActiveList.clear();
        }
    }

    /// <summary>
    /// Flushes a vector of command lists of type _Ty
    /// </summary>
    static void FlushCommandLists(
        std::vector<RHI::ICommandList*>& List,
        std::vector<bool>&               ActiveList,
        size_t                           Count)
    {
        if (Count)
        {
            std::span View(List.data(), Count);
            auto      Queue = RHI::ISwapchain::Get()->GetQueue(true);
            Queue->Upload(View);

            for (size_t i = 0; i < Count; i++)
            {
                RHI::ICommandList* CurCommandList = List.back();
                List.pop_back();
                List.push_back(CurCommandList);
            }
            ActiveList.erase(ActiveList.begin(), ActiveList.begin() + Count);
            ActiveList.insert(ActiveList.end(), Count, false);
        }
    }

    /// <summary>
    /// Reset a vector of command lists of type _Ty
    /// </summary>
    static void ResetCommandLists(
        std::vector<RHI::ICommandList*>& List,
        std::vector<bool>&               ActiveList,
        size_t                           Count)
    {
        if (Count)
        {
            std::span View(List.data(), Count);
            auto      Queue = RHI::ISwapchain::Get()->GetQueue(true);
            Queue->Reset(RHI::CommandQueueType::Graphics, View);

            for (size_t i = 0; i < Count; i++)
            {
                ActiveList[i] = true;
            }
        }
    }

    //
    RenderGraph::CommandListContext::CommandListContext(
        uint32_t MaxGraphicsCount,
        uint32_t MaxComputeCount)
    {
        m_GraphicsCommandList.resize(MaxGraphicsCount);
        m_ComputeCommandList.resize(MaxComputeCount);

        m_Fence.reset(RHI::IFence::Create());
    }

    void RenderGraph::CommandListContext::Begin()
    {
        AllocateCommandLists(m_GraphicsCommandList, m_ActiveGraphicsCommandList);
        AllocateCommandLists(m_ComputeCommandList, m_ActiveComputeCommandList);
    }

    void RenderGraph::CommandListContext::Flush(
        size_t GraphicsCount,
        size_t ComputeCount)
    {
        FlushCommandLists(m_GraphicsCommandList, m_ActiveGraphicsCommandList, GraphicsCount);
        FlushCommandLists(m_ComputeCommandList, m_ActiveComputeCommandList, ComputeCount);

        if (ComputeCount)
        {
            auto Queue = RHI::ISwapchain::Get()->GetQueue(true);
            m_Fence->SignalGPU(Queue, m_FenceValue);
            m_Fence->WaitGPU(Queue, m_FenceValue++);
        }
    }

    void RenderGraph::CommandListContext::Reset(
        size_t GraphicsCount,
        size_t ComputeCount)
    {
        ResetCommandLists(m_GraphicsCommandList, m_ActiveGraphicsCommandList, GraphicsCount);
        ResetCommandLists(m_ComputeCommandList, m_ActiveComputeCommandList, ComputeCount);
    }

    void RenderGraph::CommandListContext::End()
    {
        FreeCommandLists(m_GraphicsCommandList, m_ActiveGraphicsCommandList);
        FreeCommandLists(m_ComputeCommandList, m_ActiveComputeCommandList);
    }

    RHI::ICommandList* RenderGraph::CommandListContext::GetGraphics(
        size_t Index)
    {
        return m_GraphicsCommandList[Index];
    }

    RHI::ICommandList* RenderGraph::CommandListContext::GetCompute(
        size_t Index)
    {
        return m_ComputeCommandList[Index];
    }

    size_t RenderGraph::CommandListContext::GetGraphicsCount() const noexcept
    {
        return m_GraphicsCommandList.size();
    }

    size_t RenderGraph::CommandListContext::GetComputeCount() const noexcept
    {
        return m_ComputeCommandList.size();
    }

    //

    GraphDepdencyLevel::GraphDepdencyLevel(
        RenderGraph& Context) :
        m_Context(Context)
    {
    }

    void GraphDepdencyLevel::AddPass(
        UPtr<IRenderPass>                             Pass,
        std::vector<ResourceViewId>                   RenderTargets,
        std::optional<ResourceViewId>                 DepthStencil,
        std::set<ResourceId>                          ResourceToCreate,
        std::map<ResourceViewId, RHI::MResourceState> States)
    {
        m_Passes.emplace_back(std::move(Pass), std::move(RenderTargets), std::move(DepthStencil));
        m_ResourcesToCreate.merge(std::move(ResourceToCreate));

        auto& Storage = m_Context.GetStorage();

        for (auto& [ViewId, State] : States)
        {
            uint32_t SubresourceIndex;
            Storage.GetResourceView(ViewId, nullptr, &SubresourceIndex);
            auto& States = m_StatesToTransition[ViewId.GetResource()][SubresourceIndex];
            States |= State;
        }
    }

    std::pair<uint32_t, uint32_t> GraphDepdencyLevel::GetCommandListCount() const
    {
        uint32_t GraphicsCount = 0, ComputeCount = 0;
        for (auto& PassInfo : m_Passes)
        {
            switch (PassInfo.Pass->GetQueueType())
            {
            case PassQueueType::Direct:
                GraphicsCount++;
                break;
            case PassQueueType::Compute:;
                ComputeCount++;
                break;
            }
        }
        return { GraphicsCount, ComputeCount };
    }

    //

    void GraphDepdencyLevel::Execute() const
    {
        auto& Storage = m_Context.GetStorage();

        for (auto& PassInfo : m_Passes)
        {
            PassInfo.Pass->PreDispatch(Storage);
        }

        for (auto& Id : m_ResourcesToCreate)
        {
            auto& Handle = Storage.GetResourceMut(Id);
            if (!Handle.IsImported())
            {
                Storage.ReallocateResource(Handle);
            }
            Storage.CreateViews(Handle);
        }

        ExecuteBarriers();

        {
            auto StateManager = RHI::IResourceStateManager::Get();

            const bool UsingGraphics    = m_GraphicsCommandsToFlush > 0;
            auto       FirstCommandList = m_Context.m_CommandListContext.GetGraphics(0);

            if (m_ResetBarriers)
            {
                m_Context.m_CommandListContext.Reset(1, 0);
            }

            StateManager->FlushBarriers(FirstCommandList);

            if (m_FlushBarriers)
            {
                m_Context.m_CommandListContext.Flush(1, 0);
            }
        }

        if (m_ResetCommands)
        {
            m_Context.m_CommandListContext.Reset(m_GraphicsCommandsToReserve, m_ComputeCommandsToReserve);
        }

        ExecutePasses();

        if (m_FlushCommands)
        {
            m_Context.m_CommandListContext.Flush(m_GraphicsCommandsToFlush, m_ComputeCommandsToFlush);
        }
    }

    void GraphDepdencyLevel::ExecuteBarriers() const
    {
        auto& Storage      = m_Context.GetStorage();
        auto  StateManager = RHI::IResourceStateManager::Get();

        for (auto& [Resource, StateMap] : m_StatesToTransition)
        {
            auto& Handle = Storage.GetResource(Resource);
            for (auto& [SubresourceIndex, State] : StateMap)
            {
                StateManager->TransitionResource(
                    Handle.Get().get(),
                    State,
                    SubresourceIndex);
            }
        }
    }

    //

    void GraphDepdencyLevel::ExecutePasses() const
    {
        auto DispatchTask =
            [this](
                size_t   PassIndex,
                uint32_t CommandIndex)
        {
            auto& [RenderPass, RenderTargets, DepthStencil] = m_Passes[PassIndex];
            auto& Storage                                   = m_Context.GetStorage();

            RHI::ICommandList* CommandList = nullptr;

            switch (RenderPass->GetQueueType())
            {
            case PassQueueType::Direct:
            {
                auto RenderCommandList = m_Context.m_CommandListContext.GetGraphics(CommandIndex);

                CommandList = RenderCommandList;
                CommandList->BeginEvent(RenderPass->GetPassName());

                std::vector<RHI::CpuDescriptorHandle> RtvHandles;
                RtvHandles.reserve(RenderTargets.size());

                RHI::CpuDescriptorHandle  DsvHandle;
                RHI::CpuDescriptorHandle* DsvHandlePtr = nullptr;

                if (!RenderTargets.empty())
                {
                    CommandList->MarkEvent("ClearRenderTargets", Colors::DarkOrange);
                    for (auto& RtvViewId : RenderTargets)
                    {
                        RHI::CpuDescriptorHandle RtvHandle;

                        auto& Handle   = Storage.GetResource(RtvViewId.GetResource());
                        auto& ViewDesc = std::get<RHI::RTVDescOpt>(Storage.GetResourceView(RtvViewId, &RtvHandle));

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
                }

                if (DepthStencil)
                {
                    auto& Handle   = Storage.GetResource(DepthStencil->GetResource());
                    auto& ViewDesc = std::get<RHI::DSVDescOpt>(Storage.GetResourceView(*DepthStencil, &DsvHandle));
                    auto& Desc     = Handle.GetDesc();

                    if (ViewDesc && ViewDesc->ClearType != RHI::EDSClearType::Ignore)
                    {
                        std::optional<float>   Depth;
                        std::optional<uint8_t> Stencil;

                        auto ClearValue = Desc.ClearValue ? std::get_if<RHI::ClearOperation::DepthStencil>(&Desc.ClearValue->Value) : nullptr;
                        if (ClearValue)
                        {
                            Depth   = ClearValue->Depth;
                            Stencil = ClearValue->Stencil;
                        }

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

                        CommandList->MarkEvent("ClearDepthStencil", Colors::OrangeRed);
                        RenderCommandList->ClearDsv(DsvHandle, Depth, Stencil);
                    }

                    DsvHandlePtr = &DsvHandle;
                }

                RenderCommandList->SetRenderTargets(
                    RtvHandles.data(),
                    RtvHandles.size(),
                    DsvHandlePtr);

                if (!RenderPass->OverrideViewport(Storage, RenderCommandList))
                {
                    auto Size = Storage.GetOutputImageSize();

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
                CommandList = m_Context.m_CommandListContext.GetCompute(CommandIndex);
                CommandList->BeginEvent(RenderPass->GetPassName());
                break;
            }
            }

#ifndef NEON_DIST
            RHI::RenameObject(CommandList, StringUtils::Transform<String>(RenderPass->GetPassName()));
#endif
            RenderPass->Dispatch(Storage, CommandList);

            CommandList->EndEvent();
        };

#ifdef NEON_RENDER_GRAPH_THREADED
        std::vector<std::future<void>> Futures;
        Futures.reserve(m_Passes.size());
#endif

        uint32_t ComputeCommandIndex = 0, GraphicsCommandIndex = 0;

        for (size_t i = 0; i < m_Passes.size(); i++)
        {
            if (m_Passes[i].Pass->GetQueueType() == PassQueueType::Unknown ||
                m_Passes[i].Pass->GetPassFlags().Test(EPassFlags::Cull))
            {
                continue;
            }

            uint32_t& CommandIndex = m_Passes[i].Pass->GetQueueType() == PassQueueType::Direct ? GraphicsCommandIndex : ComputeCommandIndex;

#ifdef NEON_RENDER_GRAPH_THREADED
            Futures.emplace_back(m_Context.m_ThreadPool.enqueue(DispatchTask, i, CommandIndex));
#else
            DispatchTask(i, CommandIndex);
#endif

            CommandIndex++;
        }

#ifdef NEON_RENDER_GRAPH_THREADED
        for (auto& Future : Futures)
        {
            Future.get();
        }
#endif
    }
} // namespace Neon::RG
