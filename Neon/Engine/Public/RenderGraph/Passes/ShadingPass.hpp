#pragma once

#include <RenderGraph/Pass.hpp>
#include <RenderGraph/Graph.hpp>

namespace Neon::RG
{
    class ShadingPass : public GraphicsRenderPass<ShadingPass>
    {
        friend class RenderPass;

    public:
        ShadingPass();

    protected:
        void ResolveResources(
            ResourceResolver& Resolver) override;

        void DispatchTyped(
            const GraphStorage&      Storage,
            RHI::GraphicsCommandList CommandList);
    };
} // namespace Neon::RG