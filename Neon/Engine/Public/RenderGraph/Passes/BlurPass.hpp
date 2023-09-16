#pragma once

#include <RenderGraph/Pass.hpp>
#include <RenderGraph/Graph.hpp>

namespace Neon::Renderer
{
    class IMaterial;
}

namespace Neon::RG
{
    struct BlurPass
    {
        struct BlurPassData
        {
            String ViewName;

            ResourceId Source;
            ResourceId Output;

            RHI::SRVDescOpt SourceDesc;
            RHI::UAVDescOpt OutputDesc;
        };

        static void AddPass(
            GraphBuilder& Builder,
            BlurPassData  Data);
    };
} // namespace Neon::RG