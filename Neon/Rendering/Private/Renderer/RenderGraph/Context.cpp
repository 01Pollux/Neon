#include <EnginePCH.hpp>
#include <Renderer/RenderGraph/Builder.hpp>
#include <Renderer/RenderGraph/Context.hpp>

#include <RHI/ResourceStateTracker.hpp>
#include <RHI/RenderDevice.hpp>
#include <RHI/DisplayBuffers.hpp>

namespace Renderer::RG
{
    RenderGraphContext::RenderGraphContext() :
        ThisClass(RHI::IRenderDevice::Get()->GetDisplay())
    {
    }

    RenderGraphContext::RenderGraphContext(
        RHI::IDisplayBuffers* Display) :
        m_Storage(Display)
    {
    }

    RenderGraphBuilder RenderGraphContext::Reset()
    {
        m_Storage.Reset();
        return RenderGraphBuilder(*this);
    }

    RenderGraphStorage& RenderGraphContext::GetStorage()
    {
        return m_Storage;
    }

    const RenderGraphStorage& RenderGraphContext::GetStorage() const
    {
        return m_Storage;
    }

    void RenderGraphContext::Execute()
    {
        if (m_PipelineCreators.valid()) [[unlikely]]
        {
            m_PipelineCreators.wait();
            m_PipelineCreators = {};
        }

        for (auto& ImportedResource : m_Storage.m_ImportedResources)
        {
            auto& Handle = m_Storage.GetResource(ImportedResource);
            m_Storage.CreateViews(Handle);
        }

        for (auto& Level : m_Levels)
        {
            Level.Execute(m_Storage.m_Display);
        }

        if (auto PendingBarriers = RHI::ResourceStateTracker::Flush(); !PendingBarriers.empty())
        {
            RHI::GraphicsCommandContext Context(m_Storage.m_Display, RHI::CommandFlushFlags::ExecuteSingleOnSumbit);
            Context.PlaceBarrier(PendingBarriers.data(), PendingBarriers.size());
            Context.Sumbit();
        }

        m_Storage.FlushResources();
    }

    void RenderGraphContext::Build(
        std::vector<DepdencyLevel>&& Levels)
    {
        m_Levels = std::move(Levels);
    }

    //

    RenderGraphContext::DepdencyLevel::DepdencyLevel(
        RenderGraphContext& Context) :
        m_Context(Context)
    {
    }

    //

    void RenderGraphContext::DepdencyLevel::AddPass(
        IRenderPass::UPtr                               Pass,
        std::vector<ResourceViewId>                     RenderTargets,
        std::optional<ResourceViewId>                   DepthStencil,
        std::set<ResourceId>                            ResourceToCreate,
        std::set<ResourceId>                            ResourceToDestroy,
        std::map<ResourceViewId, D3D12_RESOURCE_STATES> States)
    {
        m_Passes.emplace_back(std::move(Pass), std::move(RenderTargets), std::move(DepthStencil));
        m_ResourcesToCreate.merge(std::move(ResourceToCreate));
        m_ResourcesToDestroy.merge(std::move(ResourceToDestroy));
        m_States.merge(std::move(States));
    }

    void RenderGraphContext::DepdencyLevel::Execute(
        RHI::IDisplayBuffers* Display)
    {
        auto& Storage = m_Context.GetStorage();

        for (auto& Id : m_ResourcesToCreate)
        {
            auto& Handle = Storage.GetResource(Id);
            Storage.RellocateResource(Handle);
            Storage.CreateViews(Handle);
        }

        ExecuteBarriers(Display);
        ExecutePasses(Display);

        for (auto& Id : m_ResourcesToDestroy)
        {
            auto& Handle = Storage.GetResource(Id);
            Storage.FreeResource(Handle);
        }
    }

    void RenderGraphContext::DepdencyLevel::ExecuteBarriers(
        RHI::IDisplayBuffers* Display)
    {
        auto& Storage = m_Context.GetStorage();

        for (auto& [ViewId, State] : m_States)
        {
            auto& Handle = Storage.GetResource(ViewId.GetResource());
            RHI::ResourceStateTracker::PlaceTransition(Handle.Get(), State, ViewId.GetSubresourceIndex());
        }

        auto PendingBarriers = RHI::ResourceStateTracker::Flush();
        if (PendingBarriers.empty())
        {
            return;
        }

        RHI::GraphicsCommandContext Context(Display, RHI::CommandFlushFlags::ExecuteSingleOnSumbit);
        Context.PlaceBarrier(PendingBarriers.data(), PendingBarriers.size());
        Context.Sumbit();
    }

    //

    struct CommandContextGuard
    {
        ~CommandContextGuard();

        void Execute(
            RHI::IDisplayBuffers* Display);

        RHI::ICommandContext* NewGraphics(
            RHI::IDisplayBuffers* Display);

        RHI::ICommandContext* NewCompute(
            RHI::IDisplayBuffers* Display);

        RHI::ICommandContext* NewCopy(
            RHI::IDisplayBuffers* Display);

    private:
        std::mutex m_ContextsMutex;

        std::vector<RHI::ICommandContext*> m_Contexts;
    };

    CommandContextGuard::~CommandContextGuard()
    {
        for (auto Context : m_Contexts)
        {
            delete Context;
        }
    }

    void CommandContextGuard::Execute(RHI::IDisplayBuffers* Display)
    {
        Display->ExecuteCommandLists(m_Contexts);
    }

    RHI::ICommandContext* CommandContextGuard::NewGraphics(RHI::IDisplayBuffers* Display)
    {
        std::lock_guard Lock(m_ContextsMutex);
        return m_Contexts.emplace_back(RHI::GraphicsCommandContext::New(Display, RHI::CommandFlushFlags::DontDefer));
    }

    RHI::ICommandContext* CommandContextGuard::NewCompute(RHI::IDisplayBuffers* Display)
    {
        std::lock_guard Lock(m_ContextsMutex);
        return m_Contexts.emplace_back(RHI::ComputeCommandContext::New(Display, RHI::CommandFlushFlags::DontDefer));
    }

    RHI::ICommandContext* CommandContextGuard::NewCopy(RHI::IDisplayBuffers* Display)
    {
        std::lock_guard Lock(m_ContextsMutex);
        return m_Contexts.emplace_back(RHI::CopyCommandContext::New(Display, RHI::CommandFlushFlags::DontDefer));
    }

    //

    void RenderGraphContext::DepdencyLevel::ExecutePasses(
        RHI::IDisplayBuffers* Display) const
    {
        CommandContextGuard       ContextsToExecute;
        std::vector<std::jthread> PassesToExecute;

        PassesToExecute.reserve(m_Passes.size());

        for (size_t i = 0; i < m_Passes.size(); i++)
        {
            PassesToExecute.emplace_back(
                [&Storage = m_Context.GetStorage(),
                 this,
                 &ContextsToExecute,
                 Display](size_t PassIndex)
                {
                    RHI::ICommandContext* Context = nullptr;

                    auto& [RenderPass, RenderTargets, DepthStencil] = m_Passes[PassIndex];

                    switch (RenderPass->GetQueueType())
                    {
                    case PassQueueType::Direct:
                    {
                        Context = ContextsToExecute.NewGraphics(Display);

                        auto& GraphicsContext = static_cast<RHI::GraphicsCommandContext&>(*Context);

                        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> RtvHandles;

                        D3D12_CPU_DESCRIPTOR_HANDLE  DsvHandle;
                        D3D12_CPU_DESCRIPTOR_HANDLE* DsvHandlePtr = nullptr;

                        for (auto& RtvViewId : RenderTargets)
                        {
                            D3D12_CPU_DESCRIPTOR_HANDLE RtvCpuHandle;

                            auto& Handle   = Storage.GetResource(RtvViewId.GetResource());
                            auto& ViewDesc = std::get<RenderTargetViewDesc>(Storage.GetResourceView(RtvViewId, &RtvCpuHandle));

                            auto& Desc = Handle.GetDesc();
                            RtvHandles.emplace_back(RtvCpuHandle);

                            if (ViewDesc.ClearType == ClearType::RTV_Color)
                            {
                                if (ViewDesc.ForceColor)
                                {
                                    GraphicsContext.ClearRtv(RtvCpuHandle, *ViewDesc.ForceColor);
                                }
                                else if (Desc.ClearValue)
                                {
                                    GraphicsContext.ClearRtv(RtvCpuHandle, Desc.ClearValue->Color);
                                }
                            }
                        }

                        if (DepthStencil)
                        {
                            auto& Handle   = Storage.GetResource(DepthStencil->GetResource());
                            auto& ViewDesc = std::get<DepthStencilViewDesc>(Storage.GetResourceView(*DepthStencil, &DsvHandle));

                            auto& Desc   = Handle.GetDesc();
                            DsvHandlePtr = &DsvHandle;

                            float             Depth      = 1.f;
                            uint8_t           Stencil    = 0;
                            D3D12_CLEAR_FLAGS ClearFlags = {};

                            switch (ViewDesc.ClearType)
                            {
                            case ClearType::DSV_Depth:
                            {
                                Depth = ViewDesc.ForceDepth ? *ViewDesc.ForceDepth : Desc.ClearValue->DepthStencil.Depth;
                                ClearFlags |= D3D12_CLEAR_FLAG_DEPTH;
                                break;
                            }
                            case ClearType::DSV_Stencil:
                            {
                                Stencil = ViewDesc.ForceStencil ? *ViewDesc.ForceStencil : Desc.ClearValue->DepthStencil.Stencil;
                                ClearFlags |= D3D12_CLEAR_FLAG_STENCIL;
                                break;
                            }
                            case ClearType::DSV_DepthStencil:
                            {
                                Depth   = ViewDesc.ForceDepth ? *ViewDesc.ForceDepth : Desc.ClearValue->DepthStencil.Depth;
                                Stencil = ViewDesc.ForceStencil ? *ViewDesc.ForceStencil : Desc.ClearValue->DepthStencil.Stencil;
                                ClearFlags |= D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL;
                                break;
                            }
                            }
                            if (ClearFlags)
                            {
                                GraphicsContext.ClearDsv(DsvHandle, ClearFlags, Depth, Stencil);
                            }
                        }

                        GraphicsContext.SetRenderTargetView(
                            RtvHandles.data(),
                            RtvHandles.size(),
                            false,
                            DsvHandlePtr);

                        if (!RenderPass->OverrideViewport(Storage, GraphicsContext))
                        {
                            auto& PassViewport    = RenderPass->GetViewport();
                            auto& DisplayViewport = Display->GetSize();
                            auto& FinalViewport   = !PassViewport.Width() ? DisplayViewport : PassViewport;

                            CD3DX12_VIEWPORT Viewport(0.f, 0.f, float(FinalViewport.Width()), float(FinalViewport.Height()));
                            CD3DX12_RECT     Rect(0, 0, FinalViewport.Width(), FinalViewport.Height());
                            GraphicsContext.SetViewports(&Viewport, 1);
                            GraphicsContext.SetScissorRects(&Rect, 1);
                        }

                        break;
                    }

                    case PassQueueType::Compute:
                    {
                        Context = ContextsToExecute.NewCompute(Display);
                        break;
                    }

                    case PassQueueType::Copy:
                    {
                        Context = ContextsToExecute.NewCopy(Display);
                        break;
                    }
                    }

                    RenderPass->Execute(Storage, *Context);
                    Context->Sumbit();
                },
                i);
        }

        PassesToExecute.clear();
        ContextsToExecute.Execute(Display);
    }
} // namespace Renderer::RG
