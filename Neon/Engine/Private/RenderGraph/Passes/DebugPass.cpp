#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>

#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/DebugPass.hpp>

namespace Neon::RG
{
    using namespace Scene;

    DebugPass::DebugPass() :
        IRenderPass("DebugPass", PassQueueType::Direct)
    {
        //
    }

    void DebugPass::ResolveResources(
        ResourceResolver& Resolver)
    {
    }

    void DebugPass::Dispatch(
        const GraphStorage& Storage,
        RHI::ICommandList*  CommandList)
    {
    }
} // namespace Neon::RG