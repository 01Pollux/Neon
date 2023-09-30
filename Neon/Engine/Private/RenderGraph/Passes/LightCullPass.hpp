#include <EnginePCH.hpp>

#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/LightCullPass.hpp>

namespace Neon::RG
{
    LightCullPass::LightCullPass() :
        RenderPass("LightCullPass")
    {
    }

    void LightCullPass::ResolveResources(
        ResourceResolver& Resolver)
    {
    }

    void LightCullPass::DispatchTyped(
        const GraphStorage&     Storage,
        RHI::ComputeCommandList CommandList)
    {
    }
} // namespace Neon::RG