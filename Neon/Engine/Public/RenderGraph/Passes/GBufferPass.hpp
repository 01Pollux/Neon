#pragma once

#include <RenderGraph/Pass.hpp>
#include <RenderGraph/Graph.hpp>
#include <Renderer/Render/BaseRenderer.hpp>

#include <Scene/Scene.hpp>

namespace Neon::RG
{
    class GBufferPass : public IRenderPass
    {
    public:
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

        GBufferPass(
            flecs::entity Camera);

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

        void Dispatch(
            const GraphStorage& Storage,
            RHI::ICommandList*  CommandList) override;

    private:
        /// <summary>
        /// Update camera buffer.
        /// </summary>
        [[nodiscard]] RHI::GpuResourceHandle UpdateCameraBuffer();

    private:
        flecs::entity m_Camera;

        std::vector<UPtr<Renderer::IRenderer>> m_Renderers;
    };
} // namespace Neon::RG