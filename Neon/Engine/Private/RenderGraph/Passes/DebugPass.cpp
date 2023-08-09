#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>
#include <Runtime/DebugOverlay.hpp>

#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/DebugPass.hpp>

namespace Neon::RG
{
    using namespace Scene;

    DebugPass::DebugPass() :
        IRenderPass("DebugPass", PassQueueType::Direct)
    {
    }

    void DebugPass::ResolveResources(
        ResourceResolver& Resolver)
    {
        Resolver.WriteRenderTarget(
            RG::ResourceViewId(STR("GBufferAlbedo"), STR("DebugPass")),
            RHI::RTVDesc{
                .ClearType = RHI::ERTClearType::Ignore });
    }

    void DebugPass::Dispatch(
        const GraphStorage& Storage,
        RHI::ICommandList*  CommandList)
    { /*
         Runtime::DebugOverlay::Render(
             dynamic_cast<RHI::IGraphicsCommandList*>(CommandList),

         );*/
    }
} // namespace Neon::RG