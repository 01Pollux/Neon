#include <EnginePCH.hpp>
#include <Renderer/RG/RG.hpp>

#include <Window/Window.hpp>
#include <RHI/Resource/State.hpp>
#include <RHI/Swapchain.hpp>

#include <Log/Logger.hpp>

namespace Neon::RG
{
    RenderGraph::RenderGraph(
        RHI::ISwapchain* Swapchain) :
        m_Storage(Swapchain)
    {
    }

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

    void RenderGraph::Run()
    {
        for (auto& ImportedResource : m_Storage.m_ImportedResources)
        {
            auto& Handle = m_Storage.GetResourceMut(ImportedResource);
            m_Storage.CreateViews(Handle);
        }

        for (auto& Level : m_Levels)
        {
            Level.Execute(m_Storage.m_Swapchain);
        }

        auto StateManager = m_Storage.m_Swapchain->GetStateManager();
        StateManager->FlushBarriers(m_Storage.m_Swapchain);

        m_Storage.FlushResources();
    }

    void RenderGraph::Build(
        std::vector<RenderGraphDepdencyLevel>&& Levels)
    {
        m_Levels = std::move(Levels);
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
        RHI::ISwapchain* Swapchain)
    {
        auto& Storage = m_Context.GetStorage();

        for (auto& Id : m_ResourcesToCreate)
        {
            auto& Handle = Storage.GetResourceMut(Id);
            Storage.RellocateResource(Handle);
            Storage.CreateViews(Handle);
        }

        ExecuteBarriers(Swapchain);
        ExecutePasses(Swapchain);

        for (auto& Id : m_ResourcesToDestroy)
        {
            auto& Handle = Storage.GetResource(Id);
            Storage.FreeResource(Handle);
        }
    }

    void RenderGraphDepdencyLevel::ExecuteBarriers(
        RHI::ISwapchain* Swapchain)
    {
        auto& Storage      = m_Context.GetStorage();
        auto  StateManager = Swapchain->GetStateManager();

        for (auto& [ViewId, State] : m_States)
        {
            auto& Handle = Storage.GetResource(ViewId.GetResource());
            StateManager->TransitionResource(
                Handle.Get().get(),
                State,
                ViewId.GetSubresourceIndex());
        }

        StateManager->FlushBarriers(Swapchain);
    }

    //

    void RenderGraphDepdencyLevel::ExecutePasses(
        RHI::ISwapchain* Swapchain) const
    {
        std::mutex RenderMutex, ComputeMutex, CopyMutex;

        RHI::TCommandContext<RHI::CommandQueueType::Graphics> RenderContext(Swapchain);
        RHI::TCommandContext<RHI::CommandQueueType::Compute>  ComputeContext(Swapchain);
        RHI::TCommandContext<RHI::CommandQueueType::Copy>     CopyContext(Swapchain);

        std::vector<std::jthread> PassesToExecute;
        PassesToExecute.reserve(m_Passes.size());

        for (size_t i = 0; i < m_Passes.size(); i++)
        {
            PassesToExecute.emplace_back(
                [&, &Storage = m_Context.GetStorage()](size_t PassIndex)
                {
                    auto& [RenderPass, RenderTargets, DepthStencil] = m_Passes[PassIndex];

                    RHI::ICommandList* CommandList;

                    switch (RenderPass->GetQueueType())
                    {
                    case PassQueueType::Direct:
                    {
                        RHI::IGraphicsCommandList* RenderCommandList;
                        {
                            std::scoped_lock Lock(RenderMutex);
                            RenderCommandList = RenderContext.Append();
                            CommandList       = RenderCommandList;
                        }

                        std::vector<RHI::CpuDescriptorHandle> RtvHandles;

                        RHI::CpuDescriptorHandle  DsvHandle;
                        RHI::CpuDescriptorHandle* DsvHandlePtr = nullptr;

                        for (auto& RtvViewId : RenderTargets)
                        {
                            RHI::CpuDescriptorHandle RtcHandle;

                            auto& Handle   = Storage.GetResource(RtvViewId.GetResource());
                            auto& ViewDesc = std::get<std::optional<RHI::RTVDesc>>(Storage.GetResourceView(RtvViewId, &RtcHandle));

                            auto& Desc = Handle.GetDesc();
                            RtvHandles.emplace_back(RtcHandle);

                            if (ViewDesc && ViewDesc->ClearType != RHI::ERTClearType::Ignore)
                            {
                                if (ViewDesc->ForceColor || Desc.ClearValue)
                                {
                                    RenderCommandList->ClearRtv(RtcHandle, ViewDesc->ForceColor.value_or(
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
                            auto Size = Swapchain->GetWindow()->GetSize();

                            RenderCommandList->SetViewport(
                                ViewportF{
                                    .Width    = float(Size.Width()),
                                    .Height   = float(Size.Height()),
                                    .MaxDepth = 1.f,
                                });
                            RenderCommandList->SetScissorRect(RectF(Vector2D::Zero, Size));
                        }

                        break;
                    }

                    case PassQueueType::Compute:
                    {
                        {
                            std::scoped_lock Lock(RenderMutex);
                            CommandList = CopyContext.Append();
                        }
                        break;
                    }

                    case PassQueueType::Copy:
                    {
                        {
                            std::scoped_lock Lock(RenderMutex);
                            CommandList = CopyContext.Append();
                        }
                        break;
                    }
                    }

                    RenderPass->Dispatch(Storage, CommandList);
                },
                i);
        }
    }
} // namespace Neon::RG
