#pragma once

#include <Config/Engine.hpp>
#include <Window/Window.hpp>
#include <World/World.hpp>
#include <Utils/Signal.hpp>

NEON_SIGNAL_DECL(OnWindowSizeChanged, const Neon::Size2I& /*NewSize*/);

namespace Neon::Module
{
    class Window
    {
    public:
        Window(
            Neon::World&                World,
            const Config::EngineConfig& Config);

        /// <summary>
        /// Get exit code.
        /// </summary>
        [[nodiscard]] int GetExitCode() const noexcept;

        /// <summary>
        /// Get associated window.
        /// </summary>
        [[nodiscard]] Windowing::IWindowApp* GetWindow() const noexcept;

    private:
        /// <summary>
        /// Called on ::PreFrame
        /// Dispatch the pending window events
        /// </summary>
        void MessageLoop(
            flecs::iter& Iter);

    public:
        NEON_SIGNAL_INST(OnWindowSizeChanged);

    private:
        UPtr<Windowing::IWindowApp> m_Window;
        int                         m_ExitCode = 0;
    };
} // namespace Neon::Module