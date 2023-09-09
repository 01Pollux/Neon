#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/SSAOPass.hpp>

namespace Neon::RG
{
    SSAOPass::SSAOPass() :
        RenderPass(STR("SSAOPass"))
    {
    }

    void SSAOPass::ResolveResources(
        ResourceResolver& Resolver)
    {
    }

    void SSAOPass::DispatchTyped(
        const GraphStorage&       Storage,
        RHI::IComputeCommandList* CommandList)
    {
    }
} // namespace Neon::RG