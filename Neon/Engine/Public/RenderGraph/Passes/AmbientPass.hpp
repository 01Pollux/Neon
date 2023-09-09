#pragma once

#include <RenderGraph/Pass.hpp>
#include <RenderGraph/Graph.hpp>

namespace Neon::RG
{
    class AmbientPass : public ComputeRenderPass<AmbientPass>
    {
        friend class RenderPass;

    public:
        AmbientPass();

    protected:
        void ResolveResources(
            ResourceResolver& Resolver) override;

        void DispatchTyped(
            const GraphStorage&       Storage,
            RHI::IComputeCommandList* CommandList);
    };
} // namespace Neon::RG