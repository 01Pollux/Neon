#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/AmbientPass.hpp>

namespace Neon::RG
{
    AmbientPass::AmbientPass() :
        RenderPass(STR("AmbientPass"))
    {
    }

    void AmbientPass::ResolveResources(
        ResourceResolver& Resolver)
    {
    }

    void AmbientPass::DispatchTyped(
        const GraphStorage&       Storage,
        RHI::ICommandList* CommandList)
    {
    }
} // namespace Neon::RG