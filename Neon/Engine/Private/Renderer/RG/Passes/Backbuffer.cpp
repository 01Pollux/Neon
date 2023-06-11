#include <EnginePCH.hpp>
#include <Renderer/RG/RG.hpp>
#include <Renderer/RG/Passes/Backbuffer.hpp>

#include <RHI/Resource/State.hpp>
#include <RHI/Swapchain.hpp>

namespace Neon::RG
{
    void InitializeBackbufferPass::ResolveResources(
        ResourceResolver& Resolver)
    {
        auto Desc = RHI::ResourceDesc::Tex2D(
            Resolver.GetSwapchainFormat(),
            0, 0, 1, 1);
        Desc.SetClearValue(Colors::Fuchsia);

        Resolver.CreateTexture(
            RG::ResourceId(STR("FinalTexture")),
            Desc,
            MResourceFlags::FromEnum(EResourceFlags::WindowSizeDependent));
    }

    //

    void FinalizeBackbufferPass::ResolveResources(
        ResourceResolver& Resolver)
    {
        Resolver.ReadSrcResource(RG::ResourceViewId(STR("FinalTexture"), STR("CopyToBackbuffer")));
    }

    void FinalizeBackbufferPass::Dispatch(
        const GraphStorage& Storage,
        RHI::ICommandList*  CommandList)
    {
        auto RenderCommandList = dynamic_cast<RHI::IGraphicsCommandList*>(CommandList);

        auto Swapchain    = Storage.GetSwapchain();
        auto StateManager = Swapchain->GetStateManager();

        auto Backbuffer   = Swapchain->GetBackBuffer();
        auto FinalTexture = Storage.GetResource(RG::ResourceId(STR("FinalTexture"))).AsTexture();

        //

        StateManager->TransitionResource(
            Backbuffer,
            RHI::MResourceState::FromEnum(RHI::EResourceState::CopyDest));

        StateManager->TransitionResource(
            FinalTexture.get(),
            RHI::MResourceState::FromEnum(RHI::EResourceState::CopySource));

        StateManager->FlushBarriers(CommandList);

        RenderCommandList->CopyResources(Backbuffer, FinalTexture.get());

        StateManager->TransitionResource(
            Backbuffer,
            RHI::MResourceState_Present);
    }
} // namespace Neon::RG
