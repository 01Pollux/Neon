#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/SSAOPass.hpp>

namespace Neon::RG
{
    SSAOPass::SSAOPass() :
        IRenderPass(STR("SSAOPass"), RG::PassQueueType::Compute)
    {
    }

    void SSAOPass::ResolveResources(
        ResourceResolver& Resolver)
    {
    }

    void SSAOPass::Dispatch(
        const GraphStorage& Storage,
        RHI::ICommandList*  CommandList)
    {
    }
} // namespace Neon::RG