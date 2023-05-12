#pragma once

#include <Renderer/RenderGraph.hpp>

namespace Renderer::RG
{
    class InitializeBackbufferPass : public IRenderPass
    {
        NEON_DECLARE_CHILDCLASS(InitializeBackbufferPass, IRenderPass);

    public:
        InitializeBackbufferPass();

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
    };

    class FinalizeBackbufferPass : public IRenderPass
    {
        NEON_DECLARE_CHILDCLASS(FinalizeBackbufferPass, IRenderPass);

    public:
        FinalizeBackbufferPass();

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
    };
} // namespace Renderer::RG
