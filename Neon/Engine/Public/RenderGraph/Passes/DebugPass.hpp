#pragma once

#include <RenderGraph/Pass.hpp>
#include <RenderGraph/Graph.hpp>

namespace Neon::RG
{
    class DebugPass : public IRenderPass
    {
    public:
        DebugPass(
            ResourceId DrawTarget);

    protected:
        void ResolveResources(
            ResourceResolver& Resolver) override;

        void Dispatch(
            const GraphStorage& Storage,
            RHI::ICommandList*  CommandList) override;

    private:
        ResourceId m_DrawTarget;
    };
} // namespace Neon::RG