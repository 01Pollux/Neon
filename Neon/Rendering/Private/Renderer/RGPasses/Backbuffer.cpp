#include <EnginePCH.hpp>
#include <Renderer/RGPasses/Backbuffer.hpp>

#include <RHI/RenderDevice.hpp>
#include <RHI/DisplayBuffers.hpp>
#include <RHI/ResourceStateTracker.hpp>

namespace Renderer::RG
{
    InitializeBackbufferPass::InitializeBackbufferPass() :
        BaseClass(PassQueueType::Direct)
    {
    }

    void InitializeBackbufferPass::SetupResources(
        RenderGraphPassResBuilder& PassBuilder)
    {
        ResourceDesc Desc = ResourceDesc::Tex2D(
            RHI::IRenderDevice::SwapchainFormat,
            0, 0, 1, 1);
        Desc.SetClearValue(
            RHI::IRenderDevice::SwapchainFormat,
            Colors::Fuchsia);

        PassBuilder.CreateTexture(
            RG_RESOURCEID("FinalTexture"),
            Desc,
            ResourceFlags::WindowSizedTexture);
    }

    void InitializeBackbufferPass::Execute(
        const RenderGraphStorage& GraphStorage,
        RHI::ICommandContext&     CmdContext)
    {
    }

    //

    FinalizeBackbufferPass::FinalizeBackbufferPass() :
        BaseClass(PassQueueType::Direct)
    {
    }

    void FinalizeBackbufferPass::SetupResources(
        RenderGraphPassResBuilder& PassBuilder)
    {
        PassBuilder.ReadSrcResource(
            RG_RESOURCEVIEW("FinalTexture", "CopyToBackbuffer"));
    }

    void FinalizeBackbufferPass::Execute(
        const RenderGraphStorage& GraphStorage,
        RHI::ICommandContext&     CmdContext)
    {
        auto& GraphicsContext = static_cast<RHI::GraphicsCommandContext&>(CmdContext);

        auto Display = GraphStorage.GetDisplay();

        auto& Backbuffer   = Display->GetCurrentBackBuffer();
        auto& FinalTexture = GraphStorage.GetResource(RG_RESOURCEID("FinalTexture")).Get();

        //

        RHI::ResourceStateTracker::PlaceTransition(Backbuffer, D3D12_RESOURCE_STATE_COPY_DEST);
        RHI::ResourceStateTracker::PlaceTransition(FinalTexture, D3D12_RESOURCE_STATE_COPY_SOURCE);

        if (auto PendingBarriers = RHI::ResourceStateTracker::Flush(); !PendingBarriers.empty())
        {
            GraphicsContext.PlaceBarrier(PendingBarriers.data(), PendingBarriers.size());
        }

        CmdContext.CopyResources(Backbuffer, FinalTexture);

        RHI::ResourceStateTracker::PlaceTransition(Backbuffer, D3D12_RESOURCE_STATE_PRESENT);
    }
} // namespace Renderer::RG
