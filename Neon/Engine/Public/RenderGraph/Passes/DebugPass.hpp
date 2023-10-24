#pragma once

#include <RenderGraph/Pass.hpp>
#include <RenderGraph/Graph.hpp>

namespace Neon::RG
{
    class DebugPass : public GraphicsRenderPass<DebugPass>
    {
        friend class RenderPass;

    public:
        DebugPass(
            ResourceId DrawTarget);

        UPtr<IRenderPass> Clone() override;

    protected:
        void ResolveResources(
            ResourceResolver& Resolver) override;

        void DispatchTyped(
            const GraphStorage& Storage,
            RHI::ICommandList*  CommandList);

    private:
        ResourceId m_DrawTarget;
    };
} // namespace Neon::RG