#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/AmbientPass.hpp>

namespace Neon::RG
{
    AmbientPass::AmbientPass() :
        IRenderPass(STR("AmbientPass"), RG::PassQueueType::Compute)
    {
    }

    void AmbientPass::ResolveResources(
        ResourceResolver& Resolver)
    {
    }

    void AmbientPass::Dispatch(
        const GraphStorage& Storage,
        RHI::ICommandList*  CommandList)
    {
    }
} // namespace Neon::RG