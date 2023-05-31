#pragma once

#include <RHI/Commands/Common.hpp>

namespace Neon::RHI
{
    class ICommandContext;
}

namespace Neon::RG
{
    class GraphStorage;

    class IRenderPass
    {
    public:
        class ShaderResolver;
        class MaterialResolver;
        class ResourceResolver;
        class PipelineResolver;

        virtual ~IRenderPass() = default;

        /// <summary>
        /// Called when the render pass wants to resolve the dependencies of resources.
        /// </summary>
        virtual void ResolveResources(
            ResourceResolver& Resolver) = 0;

        /// <summary>
        /// Called when the render pass wans to load shaders.
        /// </summary>
        virtual void ResolveShaders(
            ShaderResolver& Resolver) = 0;

        /// <summary>
        /// Called when the render pass wants to load materials.
        /// </summary>
        virtual void ResolveMaterials(
            MaterialResolver& Resolver) = 0;

        /// <summary>
        /// Called when the render pass wants to create pipelines.
        /// </summary>
        virtual void ResolvePipelines(
            PipelineResolver& Resolver) = 0;

        /// <summary>
        /// Called when the render pass wants to dispatch.
        /// </summary>
        virtual void Dispatch(
            const GraphStorage&   Storage,
            RHI::ICommandContext& CommandContext) = 0;
    };
} // namespace Neon::RG