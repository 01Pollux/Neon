#pragma once

#include <Config/Engine.hpp>
#include <Window/Window.hpp>
#include <Utils/Signal.hpp>

NEON_SIGNAL_DECL(OnWindowSizeChanged, const Neon::Size2I& /*NewSize*/);

namespace Neon
{
    class DefaultGameEngine;
} // namespace Neon

namespace Neon::Module
{
    class Graphics;

    class Window
    {
    public:
        Window(
            DefaultGameEngine*          Engine,
            const Config::EngineConfig& Config);
        NEON_CLASS_COPYMOVE(Window);
        ~Window();

        /// <summary>
        /// Get exit code.
        /// </summary>
        [[nodiscard]] int GetExitCode() const noexcept;

        /// <summary>
        /// Get associated window.
        /// </summary>
        [[nodiscard]] Windowing::IWindowApp* GetWindow() const noexcept;

        /// <summary>
        /// Get associated geraphics module.
        /// </summary>
        [[nodiscard]] Graphics* GetGraphics() const noexcept;

        /// <summary>
        /// Dispatch the pending window events
        /// Run the window message loop.
        /// </summary>
        bool Run();

    public:
        NEON_SIGNAL_INST(OnWindowSizeChanged);

    private:
        UPtr<Windowing::IWindowApp> m_Window;
        UPtr<Graphics>              m_Graphics;
        int                         m_ExitCode = 0;
    };
} // namespace Neon::Module