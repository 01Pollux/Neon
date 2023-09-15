#pragma once

#include <RenderGraph/Pass.hpp>
#include <RenderGraph/Graph.hpp>

namespace Neon::RG
{
    class AmbientPass : public ComputeRenderPass<AmbientPass>
    {
        friend class RenderPass;

        struct ParamsType
        {
            uint32_t Color;

            uint32_t DiffuseRoughnessMap;
            uint32_t EmissiveFactorMap;
            uint32_t DepthMap;
            uint32_t AmbientOcclusionMap;
        };

    public:
        AmbientPass();

    protected:
        void ResolveResources(
            ResourceResolver& Resolver) override;

        void DispatchTyped(
            const GraphStorage& Storage,
            RHI::ICommandList*  CommandList);

    private:
        Ptr<RHI::IPipelineState> m_AmbientPassPipeline;
        Ptr<RHI::IRootSignature> m_AmbientPassRootSignature;
    };
} // namespace Neon::RG