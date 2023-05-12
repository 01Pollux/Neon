#pragma once

#include <Renderer/RenderGraph.hpp>

namespace Renderer::RG
{
    class CopyToTexturePass : public IRenderPass
    {
        NEON_DECLARE_CHILDCLASS(CopyToTexturePass, IRenderPass);

    public:
        CopyToTexturePass();

        void SetupResources(
            RenderGraphPassResBuilder& PassBuilder) override;

        void SetupRootSignatures(
            RenderGraphPassRSBuilder& PassBuilder) override;

        void SetupShaders(
            RenderGraphPassShaderBuilder& PassBuilder) override;

        void SetupPipelineStates(
            RenderGraphPassPSOBuilder& PassBuilder) override;

        void Execute(
            const RenderGraphStorage& GraphStorage,
            RHI::ICommandContext&     CmdContext) override;

    public:
        enum class CopyType : uint8_t
        {
            Default,
            AlphaBlend,
            AdditiveBlend
        };

        /// <summary>
        /// Set the source texture to copy from.
        /// </summary>
        ThisClass& SetSourceTexture(
            const ResourceViewId& SourceTexture);

        /// <summary>
        /// Set the destination texture to copy to.
        /// </summary>
        ThisClass& SetDestTexture(
            DXGI_FORMAT           Format,
            const ResourceViewId& DestTexture);

        /// <summary>
        /// Set the copy type.
        /// </summary>
        ThisClass& SetCopyType(
            CopyType Type);

    private:
        ResourceViewId m_SrcTexture;
        ResourceViewId m_DstTexture;
        DXGI_FORMAT    m_DstFormat = DXGI_FORMAT_UNKNOWN;
        CopyType       m_CopyType  = CopyType::Default;
    };
} // namespace Renderer::RG
