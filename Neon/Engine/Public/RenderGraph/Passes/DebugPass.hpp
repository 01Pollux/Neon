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

    protected:
        void ResolveResources(
            ResourceResolver& Resolver) override;

        void DispatchTyped(
            const GraphStorage&        Storage,
            RHI::IGraphicsCommandList* CommandList);

    private:
        ResourceId m_DrawTarget;
    };
} // namespace Neon::RG