#pragma once

#include <RenderGraph/Pass.hpp>
#include <RenderGraph/Graph.hpp>

namespace Neon::RG
{
    class GeometryPass : public GraphicsRenderPass<GeometryPass>
    {
        friend class RenderPass;

    public:
        static inline const ResourceId ShadedImage{ "ShadedImage" };

    public:
        GeometryPass();

    protected:
        void ResolveResources(
            ResourceResolver& Resolver) override;

        void DispatchTyped(
            const GraphStorage&      Storage,
            RHI::GraphicsCommandList CommandList);
    };
} // namespace Neon::RG