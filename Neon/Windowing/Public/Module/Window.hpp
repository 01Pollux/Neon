#pragma once

#include <Window/Window.hpp>
#include <World/World.hpp>

namespace Neon::Module
{
    class Window
    {
    public:
        Window(
            Neon::World&                   World,
            const String&                  Title,
            const Size2I&                  Size,
            const Windowing::MWindowStyle& Style);

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

    private:
        UPtr<Windowing::IWindowApp> m_Window;
        int                         m_ExitCode;
    };
} // namespace Neon::Module