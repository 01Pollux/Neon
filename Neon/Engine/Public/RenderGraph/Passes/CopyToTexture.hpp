#pragma once

#include <RenderGraph/Pass.hpp>
#include <RenderGraph/Graph.hpp>

namespace Neon::Renderer
{
    class IMaterial;
}

namespace Neon::RG
{
    class CopyToTexturePass : public GraphicsRenderPass<CopyToTexturePass>
    {
        friend class RenderPass;

    public:
        enum class BlendMode : uint8_t
        {
            Opaque,
            AlphaBlend,
            Additive,

            Count
        };

        struct CopyToTextureData
        {
            StringU8 ViewName;

            ResourceId Source;
            ResourceId Destination;

            BlendMode Blend = BlendMode::Opaque;
        };

        CopyToTexturePass(
            CopyToTextureData Data);

        UPtr<IRenderPass> Clone() override;

    protected:
        void ResolveResources(
            ResourceResolver& Resolver) override;

        void DispatchTyped(
            const GraphStorage&      Storage,
            RHI::GraphicsCommandList CommandList);

    private:
        CopyToTextureData m_Data;

        Ptr<RHI::IPipelineState> m_CopyToPipeline[static_cast<size_t>(BlendMode::Count)];
    };
} // namespace Neon::RG