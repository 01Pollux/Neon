#include <EnginePCH.hpp>
#include <Renderer/RG/RG.hpp>

#include <RHI/Swapchain.hpp>
#include <RHI/Resource/State.hpp>

#include <cppcoro/sync_wait.hpp>
#include <cppcoro/async_mutex.hpp>
#include <cppcoro/schedule_on.hpp>
#include <cppcoro/when_all.hpp>

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

    void RenderGraph::Run()
    {
        for (auto& ImportedResource : m_Storage.m_ImportedResources)
        {
            auto& Handle = m_Storage.GetResourceMut(ImportedResource);
            m_Storage.CreateViews(Handle);
        }

        for (auto& Level : m_Levels)
        {
            Level.Execute(m_ThreadPool);
        }

        RHI::IResourceStateManager::Get()->FlushBarriers();

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
        cppcoro::static_thread_pool& ThreadPool) const
    {
        auto& Storage = m_Context.GetStorage();

        for (auto& Id : m_ResourcesToCreate)
        {
            auto& Handle = Storage.GetResourceMut(Id);
            Storage.RellocateResource(Handle);
            Storage.CreateViews(Handle);
        }

        ExecuteBarriers();
        ExecutePasses(ThreadPool);

        for (auto& Id : m_ResourcesToDestroy)
        {
            auto& Handle = Storage.GetResource(Id);
            Storage.FreeResource(Handle);
        }
    }

    void RenderGraphDepdencyLevel::ExecuteBarriers() const
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

    void RenderGraphDepdencyLevel::ExecutePasses(
        cppcoro::static_thread_pool& ThreadPool) const
    {
        cppcoro::async_mutex RenderMutex, ComputeMutex;

        RHI::TCommandContext<RHI::CommandQueueType::Graphics> RenderContext;
        RHI::TCommandContext<RHI::CommandQueueType::Compute>  ComputeContext;

        std::vector<cppcoro::task<void>> Tasks;
        Tasks.reserve(m_Passes.size());

        for (size_t i = 0; i < m_Passes.size(); i++)
        {
            if (m_Passes[i].Pass->GetQueueType() == PassQueueType::Unknown)
            {
                continue;
            }

            auto Task = [&, &Storage = m_Context.GetStorage()](size_t PassIndex) -> cppcoro::task<void>
            {
                auto& [RenderPass, RenderTargets, DepthStencil] = m_Passes[PassIndex];

                RHI::ICommandList* CommandList = nullptr;

                switch (RenderPass->GetQueueType())
                {
                case PassQueueType::Direct:
                {
                    RHI::IGraphicsCommandList* RenderCommandList;
                    {
                        auto Lock         = co_await RenderMutex.scoped_lock_async();
                        RenderCommandList = RenderContext.Append();
                        CommandList       = RenderCommandList;
                    }

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
                        auto Lock   = co_await ComputeMutex.scoped_lock_async();
                        CommandList = ComputeContext.Append();
                    }
                    break;
                }
                }

                RenderPass->Dispatch(Storage, CommandList);

                co_return;
            };

            Tasks.emplace_back(cppcoro::schedule_on(ThreadPool, Task(i)));
        }

        cppcoro::sync_wait(cppcoro::when_all(std::move(Tasks)));
    }
} // namespace Neon::RG
