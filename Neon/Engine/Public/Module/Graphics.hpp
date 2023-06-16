#pragma once

#include <Config/Engine.hpp>
#include <Module/Window.hpp>

#include <RHI/Device.hpp>
#include <RHI/Swapchain.hpp>

#include <Renderer/RG/RG.hpp>

namespace Neon::Runtime
{
    class DefaultGameEngine;
} // namespace Neon::Runtime

namespace Neon::Module
{
    class Window;

    class Graphics
    {
    public:
        Graphics(
            Runtime::DefaultGameEngine* Engine,
            const Config::EngineConfig& Config,
            Window*                     WindowModule);

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

    private:
        RHI::IRenderDevice::Instance m_RenderDevice;

        UPtr<RHI::ISwapchain> m_Swapchain;
        UPtr<RG::RenderGraph> m_RenderGraph;

        Signals::SHOnWindowSizeChanged m_OnWindowSizeChanged;
    };
} // namespace Neon::Module