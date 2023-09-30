#include <EnginePCH.hpp>

#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/GridFrustumPass.hpp>

namespace Neon::RG
{
    GridFrustumPass::GridFrustumPass() :
        RenderPass("GridFrustumPass")
    {
    }

    void GridFrustumPass::ResolveResources(
        ResourceResolver& Resolver)
    {
        /* auto Desc = RHI::ResourceDesc::Buffer(
             Resolver.GetSwapchainFormat(),
             0, 0, 1, 1);

         const ResourceId GridFrustum("GridFrustum");

         Resolver.CreateWindowTexture(
             GridFrustum,
             Desc);*/
    }

    void GridFrustumPass::DispatchTyped(
        const GraphStorage&     Storage,
        RHI::ComputeCommandList CommandList)
    {
        RecreateGridFrustum();
    }

    void GridFrustumPass::RecreateGridFrustum()
    {
    }
} // namespace Neon::RG