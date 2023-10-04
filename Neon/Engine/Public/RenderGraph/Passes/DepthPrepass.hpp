#pragma once

#include <RenderGraph/Pass.hpp>
#include <RenderGraph/Graph.hpp>

namespace Neon::RG
{
    class DepthPrepass : public GraphicsRenderPass<DepthPrepass>
    {
        friend class RenderPass;

    public:
        static inline const ResourceId DepthBuffer{ "DepthBuffer" };

    public:
        DepthPrepass();

    protected:
        void ResolveResources(
            ResourceResolver& Resolver) override;

        void DispatchTyped(
            const GraphStorage&      Storage,
            RHI::GraphicsCommandList CommandList);
    };
} // namespace Neon::RG