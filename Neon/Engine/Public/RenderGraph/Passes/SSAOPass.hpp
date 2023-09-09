#pragma once

#include <RenderGraph/Pass.hpp>
#include <RenderGraph/Graph.hpp>

namespace Neon::RG
{
    class SSAOPass : public ComputeRenderPass<SSAOPass>
    {
        friend class RenderPass;

    public:
        SSAOPass();

    protected:
        void ResolveResources(
            ResourceResolver& Resolver) override;

        void DispatchTyped(
            const GraphStorage&       Storage,
            RHI::IComputeCommandList* CommandList);
    };
} // namespace Neon::RG