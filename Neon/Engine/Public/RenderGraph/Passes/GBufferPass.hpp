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
            Normal,
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
            RHI::EResourceFormat::R8G8B8A8_UNorm, // Albedo
            RHI::EResourceFormat::R8G8B8A8_UNorm, // Normal
            RHI::EResourceFormat::R8G8B8A8_UNorm, // Emissive,
            RHI::EResourceFormat::R32_Typeless    // Depth-Stencil
        };

        static constexpr RHI::EResourceFormat RenderTargetsFormatsTyped[] = {
            RHI::EResourceFormat::R8G8B8A8_UNorm, // Albedo
            RHI::EResourceFormat::R8G8B8A8_UNorm, // Normal
            RHI::EResourceFormat::R8G8B8A8_UNorm, // Emissive,
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
            const GraphStorage&        Storage,
            RHI::IGraphicsCommandList* CommandList);

    private:
        std::vector<UPtr<Renderer::IRenderer>> m_Renderers;
    };
} // namespace Neon::RG