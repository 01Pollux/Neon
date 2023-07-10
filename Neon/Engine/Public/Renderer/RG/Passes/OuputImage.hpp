#pragma once

#include <Renderer/RG/Pass.hpp>

namespace Neon::RG
{
    class InitializeOutputImage : public IRenderPass
    {
    public:
        InitializeOutputImage() :
            IRenderPass(PassQueueType::Unknown)
        {
        }

        void ResolveResources(
            ResourceResolver& Resolver) override;
    };
} // namespace Neon::RG