#pragma once

#include <RenderGraph/Pass.hpp>
#include <RenderGraph/Graph.hpp>
#include <Renderer/Render/BaseRenderer.hpp>

namespace Neon::RG
{
    class DepthPrepass : public GraphicsRenderPass<DepthPrepass>
    {
        friend class RenderPass;

    public:
        DepthPrepass();

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