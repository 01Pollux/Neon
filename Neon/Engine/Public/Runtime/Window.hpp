#pragma once

#include <Config/Engine.hpp>
#include <Window/Window.hpp>
#include <Utils/Signal.hpp>

NEON_SIGNAL_DECL(OnWindowSizeChanged, const Neon::Size2I& /*NewSize*/);

namespace Neon::Runtime
{
    class DefaultGameEngine;

    class EnginetWindow
    {
        friend class DefaultGameEngine;

    public:
        EnginetWindow(
            Runtime::DefaultGameEngine* Engine,
            const Config::EngineConfig& Config);

        /// <summary>
        /// Get exit code.
        /// </summary>
        [[nodiscard]] int GetExitCode() const noexcept;

        /// <summary>
        /// Get associated window.
        /// </summary>
        [[nodiscard]] Windowing::IWindowApp* GetWindow() const noexcept;

    protected:
        /// <summary>
        /// Dispatch the pending window events
        /// Run the window message loop.
        /// </summary>
        bool Run();

    public:
        NEON_SIGNAL_INST(OnWindowSizeChanged);

    private:
        UPtr<Windowing::IWindowApp> m_Window;

        int m_ExitCode = 0;
    };
} // namespace Neon::Runtime