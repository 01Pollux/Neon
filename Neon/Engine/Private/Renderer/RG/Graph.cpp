#include <EnginePCH.hpp>
#include <Renderer/RG/RG.hpp>
#include <Runtime/DebugOverlay.hpp>

#include <RHI/Swapchain.hpp>
#include <RHI/Resource/State.hpp>

#include <Log/Logger.hpp>

namespace Neon::RG
{
    RenderGraphBuilder RenderGraph::Reset()
    {
        m_Storage.Reset();
        return RenderGraphBuilder(*this);
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
            // We will cache the command contexts to avoid submitting single command list per pass + barrier flush
            RenderCommandContext  RenderContext;
            ComputeCommandContext ComputeContext;

            for (auto& Level : m_Levels)
            {
                Level.Execute(RenderContext, ComputeContext);
            }

            if (!RenderContext.Size() && !ComputeContext.Size())
            {
                RenderContext.Append();
            }

            if (CopyToBackBuffer)
            {
                SubmitToBackBuffer(RenderContext[0], CameraBuffer);
            }
        }
        m_Storage.FlushResources();
    }

    void RenderGraph::Build(
        std::vector<RenderGraphDepdencyLevel>&& Levels)
    {
        m_Levels = std::move(Levels);
    }

    void RenderGraph::SubmitToBackBuffer(
        RHI::IGraphicsCommandList* CommandList,
        RHI::GpuResourceHandle     CameraBuffer)
    {
        auto OutputImage = m_Storage.GetResource(RG::ResourceId(STR("OutputImage"))).AsTexture();

        auto Backbuffer   = RHI::ISwapchain::Get()->GetBackBuffer();
        auto StateManager = RHI::IResourceStateManager::Get();

        //

        RHI::TCommandContext<RHI::CommandQueueType::Graphics> CommandContext;

#ifndef NEON_DIST
        if (Runtime::DebugOverlay::ShouldRender())
        {
            // Transition the output image to a render target.
            StateManager->TransitionResource(
                OutputImage.get(),
                RHI::MResourceState::FromEnum(RHI::EResourceState::RenderTarget));

            // Prepare the command list.
            StateManager->FlushBarriers(CommandList);

            auto& Size = OutputImage->GetDimensions();

            // Set viewport and scissor rect.
            CommandList->SetViewport(ViewportF{ .Width = float(Size.x), .Height = float(Size.y) });
            CommandList->SetScissorRect(RectF({}, Size));

            // Render debug overlay.
            Runtime::DebugOverlay::Render(CommandList, CameraBuffer);
        }
#endif

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

    RenderGraphDepdencyLevel::RenderGraphDepdencyLevel(
        RenderGraph& Context) :
        m_Context(Context)
    {
    }

    //

    void RenderGraphDepdencyLevel::AddPass(
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

    void RenderGraphDepdencyLevel::Execute(
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

    void RenderGraphDepdencyLevel::ExecuteBarriers(
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

        if (!RenderContext.Size())
        {
            RenderContext.Append();
        }
        StateManager->FlushBarriers(RenderContext[0]);
    }

    //

    void RenderGraphDepdencyLevel::ExecutePasses(
        RenderGraph::RenderCommandContext&  RenderContext,
        RenderGraph::ComputeCommandContext& ComputeContext) const
    {
        auto DispatchTask = [this,
                             &RenderContext,
                             &ComputeContext](size_t PassIndex)
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
                        if (ViewDesc->ForceColor || Desc.ClearValue)
                        {
                            RenderCommandList->ClearRtv(RtvHandle, ViewDesc->ForceColor.value_or(
                                                                       std::get<Color4>(Desc.ClearValue->Value)));
                        }
                        else
                        {
                            NEON_WARNING("RenderGraph", "Render target view has no clear value, while clear type is not set to Ignore");
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

                    auto& ClearValue = std::get<RHI::ClearOperation::DepthStencil>(Desc.ClearValue->Value);

                    if (ViewDesc)
                    {
                        switch (ViewDesc->ClearType)
                        {
                        case RHI::EDSClearType::Depth:
                        {
                            Depth = ViewDesc->ForceDepth.value_or(ClearValue.Depth);
                            break;
                        }
                        case RHI::EDSClearType::Stencil:
                        {
                            Stencil = ViewDesc->ForceStencil.value_or(ClearValue.Stencil);
                            break;
                        }
                        case RHI::EDSClearType::DepthStencil:
                        {
                            Depth   = ViewDesc->ForceDepth.value_or(ClearValue.Depth);
                            Stencil = ViewDesc->ForceStencil.value_or(ClearValue.Stencil);
                            break;
                        }

                        default:
                            std::unreachable();
                        }
                    }
                    else if (Desc.ClearValue)
                    {
                        Depth   = ClearValue.Depth;
                        Stencil = ClearValue.Stencil;
                    }
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
                    std::scoped_lock Lock(m_Context.m_ComputeMutex);
                    CommandList = ComputeContext.Append();
                }
                break;
            }
            }

            RenderPass->Dispatch(Storage, CommandList);
        };

        // If we have more than one pass, we need to synchronize them
        // therefore we need to flush the chained command list we previously created
        bool ShouldFlush = m_Passes.size() > 1 || m_Passes[0].Pass->GetQueueType() == PassQueueType::Compute;
        if (ShouldFlush)
        {
            if (RenderContext.Size())
            {
                RenderContext.Upload();
            }

            std::vector<std::future<void>> Futures;
            Futures.reserve(m_Passes.size());

            for (size_t i = 0; i < m_Passes.size(); i++)
            {
                if (m_Passes[i].Pass->GetQueueType() == PassQueueType::Unknown)
                {
                    continue;
                }

                Futures.emplace_back(m_Context.m_ThreadPool.enqueue(DispatchTask, i));
            }

            for (auto& Future : Futures)
            {
                Future.get();
            }

            if (RenderContext.Size())
            {
                RenderContext.Upload();
            }
            if (ComputeContext.Size())
            {
                ComputeContext.Upload();
            }
        }
        // Here we can just execute in the current thread rather than spawning a new one
        else
        {
            DispatchTask(0);
        }
    }
} // namespace Neon::RG
