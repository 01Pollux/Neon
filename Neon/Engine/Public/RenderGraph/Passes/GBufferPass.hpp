#pragma once

#include <RenderGraph/Pass.hpp>
#include <RenderGraph/Graph.hpp>
#include <Renderer/Render/BaseRenderer.hpp>

namespace Neon::RG
{
    class GBufferPass : public GraphicsRenderPass<GBufferPass>
    {
        friend class RenderPass;

    public:
        enum class ResourceType : uint8_t
        {
            Albedo,
            Roughness = Albedo,

            Normal,
            Metallic = Normal,

            Emissive,
            DepthStencil
        };

        static constexpr const wchar_t* ResourceNames[] = {
            STR("GBufferAlbedo"),
            STR("GBufferNormal"),
            STR("GBufferEmissive"),
            STR("GBufferDepth")
        };

        static constexpr RHI::EResourceFormat RenderTargetsFormats[] = {
            RHI::EResourceFormat::R16G16B16A16_Float, // Albedo
            RHI::EResourceFormat::R16G16B16A16_Float, // Normal
            RHI::EResourceFormat::R16G16B16A16_Float, // Emissive,
            RHI::EResourceFormat::R32_Typeless    // Depth-Stencil
        };

        static constexpr RHI::EResourceFormat RenderTargetsFormatsTyped[] = {
            RHI::EResourceFormat::R16G16B16A16_Float, // Albedo
            RHI::EResourceFormat::R16G16B16A16_Float, // Normal
            RHI::EResourceFormat::R16G16B16A16_Float, // Emissive,
            RHI::EResourceFormat::D32_Float       // Depth-Stencil
        };

        /// <summary>
        /// Get the resource id of the specified resource type.
        /// </summary>
        static constexpr ResourceId GetResource(
            ResourceType Type)
        {
            return ResourceId(ResourceNames[uint8_t(Type)]);
        }

    public:
        GBufferPass();

        /// <summary>
        /// Attach a renderer to the scene pass.
        /// </summary>
        template<typename _Ty, typename... _Args>
            requires std::derived_from<_Ty, Renderer::IRenderer>
        _Ty& AttachRenderer(
            _Args&&... Args)
        {
            return *static_cast<_Ty*>(m_Renderers.emplace_back(std::make_unique<_Ty>(std::forward<_Args>(Args)...)).get());
        }

    protected:
        void ResolveResources(
            ResourceResolver& Resolver) override;

        void DispatchTyped(
            const GraphStorage&      Storage,
            RHI::GraphicsCommandList CommandList);

    private:
        std::vector<UPtr<Renderer::IRenderer>> m_Renderers;
    };
} // namespace Neon::RG