#pragma once

#include <RenderGraph/Pass.hpp>
#include <RenderGraph/Graph.hpp>

namespace Neon::RG
{
    class LightCullPass : public ComputeRenderPass<LightCullPass>
    {
        friend class RenderPass;

    public:
        LightCullPass();

    protected:
        void ResolveResources(
            ResourceResolver& Resolver) override;

        void DispatchTyped(
            const GraphStorage&     Storage,
            RHI::ComputeCommandList CommandList);
    };
} // namespace Neon::RG