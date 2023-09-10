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

        auto Rotation = Transform.World.GetRotation();
        auto Forward  = Rotation * Vec::Forward<Vector3>;
        auto Up       = Rotation * Vec::Up<Vector3>;

        auto View = glm::lookAt(
            Transform.World.GetPosition(),
            Transform.World.GetPosition() + Forward,
            Up);

        CameraBuffer.World = Transform.World.ToMat4x4Transposed();

        CameraBuffer.View           = glm::transpose(View);
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

    void RenderGraph::Draw()
    {
        m_Storage.NewOutputImage();

        for (auto& ImportedResource : m_Storage.m_ImportedResources)
        {
            auto& Handle = m_Storage.GetResourceMut(ImportedResource);
            m_Storage.CreateViews(Handle);
        }

        m_CommandListContext.Begin();

        for (size_t i = 0; i < m_Levels.size(); i++)
        {
            m_Levels[i].Execute((i + 1) != m_Levels.size());
        }

        m_CommandListContext.End();

        m_Storage.FlushResources();
    }

    void RenderGraph::Build(
        std::vector<GraphDepdencyLevel>&& Levels)
    {
        m_Levels = std::move(Levels);

        uint32_t MaxGraphics = 0, MaxCompute = 0;
        for (size_t i = 0; i < m_Levels.size(); i++)
        {
            auto ThisLevel = &m_Levels[i];
            auto NextLevel = (i + 1 < m_Levels.size()) ? &m_Levels[i + 1] : nullptr;

            bool ShouldNotFlush =
                NextLevel &&
                ThisLevel->m_Passes.size() == 1 &&
                NextLevel->m_Passes.size() == 1 &&
                ThisLevel->m_Passes[0].Pass->GetQueueType() == NextLevel->m_Passes[0].Pass->GetQueueType();

            // The only case where we should not flush is that when we have two passes in a row that are on the same queue
            ThisLevel->m_FlushCommands = !ShouldNotFlush;

            MaxGraphics = std::max(MaxGraphics, ThisLevel->m_GraphicsCount);
            MaxCompute  = std::max(MaxCompute, ThisLevel->m_ComputeCount);
        }

        m_CommandListContext = CommandListContext(MaxGraphics, MaxCompute);
    }

    //

    /// <summary>
    /// Allocates a vector of command lists of type _Ty
    /// </summary>
    template<RHI::CommandQueueType _Queue, typename _Ty>
    static void AllocateCommandLists(
        std::vector<_Ty>& List)
    {
        if (!List.empty())
        {
            auto Queue    = RHI::ISwapchain::Get()->GetQueue(_Queue);
            auto Commands = Queue->AllocateCommandLists(_Queue, List.size());
            std::transform(Commands.begin(), Commands.end(), List.begin(), [](auto& Command)
                           { return dynamic_cast<_Ty>(Command); });
        }
    }

    /// <summary>
    /// Flushes a vector of command lists of type _Ty
    /// </summary>
    template<RHI::CommandQueueType _Queue, typename _Ty>
    static void FreeCommandLists(
        const std::vector<_Ty>& List)
    {
        if (List.size())
        {
            auto Queue = RHI::ISwapchain::Get()->GetQueue(_Queue);

            std::vector<RHI::ICommandList*> Commands;
            Commands.reserve(List.size());
            std::transform(List.begin(), List.end(), std::back_inserter(Commands), [](auto& Command)
                           { return dynamic_cast<RHI::ICommandList*>(Command); });
            Queue->FreeCommandLists(_Queue, Commands);
        }
    }

    /// <summary>
    /// Flushes and reset a vector of command lists of type _Ty
    /// </summary>
    template<RHI::CommandQueueType _Queue, typename _Ty>
    static void FlushCommandLists(
        const std::vector<_Ty*>& List,
        size_t                   Count,
        bool                     Reset)
    {
        if (Count)
        {
            auto Queue = RHI::ISwapchain::Get()->GetQueue(_Queue);

            std::vector<RHI::ICommandList*> Commands;
            Commands.reserve(Count);
            std::transform(List.begin(), List.begin() + Count, std::back_inserter(Commands), [](auto& Command)
                           { return dynamic_cast<RHI::ICommandList*>(Command); });
            Queue->Upload(Commands);
            if (Reset)
            {
                Queue->Reset(_Queue, Commands);
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
        AllocateCommandLists<RHI::CommandQueueType::Graphics>(m_GraphicsCommandList);
        AllocateCommandLists<RHI::CommandQueueType::Compute>(m_ComputeCommandList);
    }

    void RenderGraph::CommandListContext::Flush(
        size_t GraphicsCount,
        size_t ComputeCount,
        bool   Reset)
    {
        FlushCommandLists<RHI::CommandQueueType::Graphics>(m_GraphicsCommandList, GraphicsCount, Reset);
        FlushCommandLists<RHI::CommandQueueType::Compute>(m_ComputeCommandList, ComputeCount, Reset);
    }

    void RenderGraph::CommandListContext::End()
    {
        FreeCommandLists<RHI::CommandQueueType::Graphics>(m_GraphicsCommandList);
        FreeCommandLists<RHI::CommandQueueType::Compute>(m_ComputeCommandList);
    }

    RHI::IGraphicsCommandList* RenderGraph::CommandListContext::GetGraphics(
        size_t Index)
    {
        return m_GraphicsCommandList[Index];
    }

    RHI::IComputeCommandList* RenderGraph::CommandListContext::GetCompute(
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

    void RenderGraph::CommandListContext::Wait(
        size_t Value)
    {
        for (auto Type : {
                 RHI::CommandQueueType::Graphics,
                 RHI::CommandQueueType::Compute })
        {
            auto Queue = RHI::ISwapchain::Get()->GetQueue(Type);
            m_Fence->WaitGPU(Queue, Value);
        }
    }

    void RenderGraph::CommandListContext::Signal(
        size_t Value)
    {
        for (auto Type : {
                 RHI::CommandQueueType::Graphics,
                 RHI::CommandQueueType::Compute })
        {
            auto Queue = RHI::ISwapchain::Get()->GetQueue(Type);
            m_Fence->SignalGPU(Queue, Value);
        }
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
        std::set<ResourceId>                          ResourceToDestroy,
        std::map<ResourceViewId, RHI::MResourceState> States)
    {
        switch (Pass->GetQueueType())
        {
        case PassQueueType::Direct:
            m_GraphicsCount++;
            break;
        case PassQueueType::Compute:;
            m_ComputeCount++;
            break;
        }

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
        bool Reset) const
    {
        auto& Storage = m_Context.GetStorage();

        for (auto& Id : m_ResourcesToCreate)
        {
            auto& Handle = Storage.GetResourceMut(Id);
            Storage.ReallocateResource(Handle);
            Storage.CreateViews(Handle);
        }

        for (auto& PassInfo : m_Passes)
        {
            PassInfo.Pass->PreDispatch(Storage);
        }

        ExecuteBarriers();

        {
            auto StateManager = RHI::IResourceStateManager::Get();

            RHI::ICommonCommandList* FirstCommandList = nullptr;

            bool UsingGraphics = m_Context.m_CommandListContext.GetGraphicsCount() > 0;
            if (UsingGraphics)
            {
                FirstCommandList = m_Context.m_CommandListContext.GetGraphics(0);
                StateManager->FlushBarriers(FirstCommandList);
                if (m_FlushCommands)
                {
                    m_Context.m_CommandListContext.Flush(1, 0, true);
                }
            }
            else
            {
                FirstCommandList = m_Context.m_CommandListContext.GetCompute(0);
                StateManager->FlushBarriers(FirstCommandList);
                if (m_FlushCommands)
                {
                    m_Context.m_CommandListContext.Flush(0, 1, true);
                }
            }
        }

        ExecutePasses();

        if (m_FlushCommands)
        {
            m_Context.m_CommandListContext.Flush(m_GraphicsCount, m_ComputeCount, Reset);
        }

        for (auto& Id : m_ResourcesToDestroy)
        {
            auto& Handle = Storage.GetResource(Id);
            if (!Handle.IsImported())
            {
                Storage.FreeResource(Handle);
            }
        }
    }

    void GraphDepdencyLevel::ExecuteBarriers() const
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

        RHI::ICommonCommandList* FirstCommandList = nullptr;
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
                CommandList            = RenderCommandList;

                std::vector<RHI::CpuDescriptorHandle> RtvHandles;
                RtvHandles.reserve(RenderTargets.size());

                RHI::CpuDescriptorHandle  DsvHandle;
                RHI::CpuDescriptorHandle* DsvHandlePtr = nullptr;

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

                if (DepthStencil)
                {
                    auto& Handle   = Storage.GetResource(DepthStencil->GetResource());
                    auto& ViewDesc = std::get<RHI::DSVDescOpt>(Storage.GetResourceView(*DepthStencil, &DsvHandle));
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
                CommandList = m_Context.m_CommandListContext.GetCompute(CommandIndex);
                break;
            }
            }

#ifndef NEON_DIST
            RHI::RenameObject(CommandList, RenderPass->GetPassName());
#endif
            RenderPass->Dispatch(Storage, CommandList);
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
