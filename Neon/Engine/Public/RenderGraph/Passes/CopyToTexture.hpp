#pragma once

#include <RenderGraph/Pass.hpp>
#include <RenderGraph/Graph.hpp>

namespace Neon::Renderer
{
    class IMaterial;
}

namespace Neon::RG
{
    class CopyToTexturePass : public IRenderPass
    {
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
            String ViewName;

            ResourceId Source;
            ResourceId Destination;

            BlendMode Blend = BlendMode::Opaque;
        };

        CopyToTexturePass(
            CopyToTextureData Data);

    protected:
        void ResolveResources(
            ResourceResolver& Resolver) override;

        void Dispatch(
            const GraphStorage& Storage,
            RHI::ICommandList*  CommandList) override;

    private:
        CopyToTextureData m_Data;

        Ptr<Renderer::IMaterial> m_Material[static_cast<size_t>(BlendMode::Count)];
    };
} // namespace Neon::RG