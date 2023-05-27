#pragma once

#include <Config/Engine.hpp>
#include <RHI/Swapchain.hpp>
#include <Module/Window.hpp>

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

        NEON_CLASS_NO_COPYMOVE(Graphics);

        ~Graphics();

        /// <summary>
        /// Get the swapchain.
        /// </summary>
        [[nodiscard]] RHI::ISwapchain* GetSwapchain() const noexcept;

        /// <summary>
        /// Prepare frame for rendering.
        /// </summary>
        void PreRender();

        /// <summary>
        /// Present frame.
        /// </summary>
        void PostRender();

    private:
        UPtr<RHI::ISwapchain> m_Swapchain;

        Signals::SHOnWindowSizeChanged m_OnWindowSizeChanged;
    };
} // namespace Neon::Module