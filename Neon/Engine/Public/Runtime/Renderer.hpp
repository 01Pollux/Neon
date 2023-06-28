#pragma once

#include <Config/Engine.hpp>
#include <Runtime/Window.hpp>

#include <RHI/Device.hpp>
#include <RHI/Swapchain.hpp>

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

        /// <summary>
        /// Get the swapchain.
        /// </summary>
        [[nodiscard]] RHI::ISwapchain* GetSwapchain() noexcept;

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
        RHI::IRenderDevice::Instance m_RenderDevice;

        Ptr<EnginetWindow> m_Window;
        bool               m_WindowIsVisible = false;

        UPtr<RHI::ISwapchain> m_Swapchain;
        UPtr<RG::RenderGraph> m_RenderGraph;

        Signals::SHOnWindowSizeChanged m_OnWindowSizeChanged;
    };
} // namespace Neon::Runtime