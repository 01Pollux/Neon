#pragma once

#include <Renderer/RG/Pass.hpp>

namespace Neon::RG
{
    class InitializeBackbufferPass : public IRenderPass
    {
    public:
        InitializeBackbufferPass() :
            IRenderPass(PassQueueType::Direct)
        {
        }

        void ResolveResources(
            ResourceResolver& Resolver) override;
    };

    class FinalizeBackbufferPass : public IRenderPass
    {
    public:
        FinalizeBackbufferPass() :
            IRenderPass(PassQueueType::Direct)
        {
        }

        void ResolveResources(
            ResourceResolver& Resolver) override;

        void Dispatch(
            const GraphStorage& Storage,
            RHI::ICommandList*  CommandList) override;
    };
} // namespace Neon::RG