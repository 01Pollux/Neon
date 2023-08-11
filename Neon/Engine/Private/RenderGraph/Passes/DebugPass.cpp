#include <EnginePCH.hpp>
#include <Runtime/DebugOverlay.hpp>

#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/DebugPass.hpp>

namespace Neon::RG
{
    DebugPass::DebugPass(
        ResourceId DrawTarget) :
        IRenderPass(STR("DebugPass"), PassQueueType::Direct),
        m_DrawTarget(std::move(DrawTarget))
    {
    }

    void DebugPass::ResolveResources(
        ResourceResolver& Resolver)
    {
        Resolver.ReadResourceEmpty(m_DrawTarget);
        Resolver.WriteRenderTarget(
            m_DrawTarget.CreateView(STR("DebugPass")),
            RHI::RTVDesc{
                .View      = RHI::RTVDesc::Texture2D{},
                .ClearType = RHI::ERTClearType::Ignore });
    }

    void DebugPass::Dispatch(
        const GraphStorage& Storage,
        RHI::ICommandList*  CommandList)
    {
        Runtime::DebugOverlay::Render(
            dynamic_cast<RHI::IGraphicsCommandList*>(CommandList),
            Storage.GetFrameDataHandle());
    }
} // namespace Neon::RG