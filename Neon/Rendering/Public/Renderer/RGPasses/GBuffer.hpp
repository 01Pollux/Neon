#pragma once

#include <Renderer/RenderGraph.hpp>

#include <Renderer/Material/Material.hpp>

namespace Renderer::RG
{
    class GBufferPass : public IRenderPass
    {
        NEON_DECLARE_CHILDCLASS(GBufferPass, IRenderPass);

    public:
        GBufferPass();

        void SetupResources(
            RenderGraphPassResBuilder& PassBuilder) override;

        void SetupRootSignatures(
            RenderGraphPassRSBuilder& PassBuilder) override
        {
        }

        void SetupShaders(
            RenderGraphPassShaderBuilder& PassBuilder) override
        {
        }

        void SetupPipelineStates(
            RenderGraphPassPSOBuilder& PassBuilder) override
        {
        }

        void Execute(
            const RenderGraphStorage& GraphStorage,
            RHI::ICommandContext&     CmdContext) override;

    private:
        Material m_Material;
    };
} // namespace Renderer::RG
