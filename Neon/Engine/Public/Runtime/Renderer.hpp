#pragma once

#include <Config/Engine.hpp>
#include <Runtime/Window.hpp>

#include <Renderer/RG/RG.hpp>

namespace Neon::Runtime
{
    class DefaultGameEngine;
    class Window;

    class EngineRenderer
    {
    public:
        EngineRenderer(
            Runtime::DefaultGameEngine* Engine,
            const Config::EngineConfig& Config);

        NEON_CLASS_NO_COPYMOVE(EngineRenderer);

        ~EngineRenderer();

        /// <summary>
        /// Get the render graph.
        /// </summary>
        [[nodiscard]] RG::RenderGraph* GetRenderGraph() noexcept;

        /// <summary>
        /// Prepare frame for rendering.
        /// </summary>
        void PreRender();

        /// <summary>
        /// Render frame.
        /// </summary>
        void Render();

        /// <summary>
        /// Present frame.
        /// </summary>
        void PostRender();

        /// <summary>
        /// Check if the renderer is rendering ie the window is visible.
        /// </summary>
        [[nodiscard]] bool IsRendering() const noexcept;

    private:
        Ptr<EngineWindow> m_Window;
        bool              m_WindowIsVisible = false;

        UPtr<RG::RenderGraph>          m_RenderGraph;
        Signals::SHOnWindowSizeChanged m_OnWindowSizeChanged;
    };
} // namespace Neon::Runtime