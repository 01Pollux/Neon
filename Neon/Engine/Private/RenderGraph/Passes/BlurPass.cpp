#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/BlurPass.hpp>

namespace Neon::RG
{
    BlurPass::BlurPass(
        BlurPassData Data) :
        RenderPass("BlurPass")
    {
    }

    void BlurPass::ResolveResources(
        ResourceResolver& Resolver)
    {
    }

    void BlurPass::DispatchTyped(
        const GraphStorage&     Storage,
        RHI::ComputeCommandList CommandList)
    {
    }
} // namespace Neon::RG