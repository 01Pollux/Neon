#pragma once

#include <Window/Window.hpp>
#include <flecs/flecs.h>

namespace flecs
{
    struct world;
}

namespace Neon::Module
{
    class Window
    {
    public:
        Window(flecs::world&)
        {
        }
        explicit Window(
            flecs::world&                  World,
            const String&                  Title,
            const Size2I&                  Size,
            const Windowing::MWindowStyle& Style);

        /// <summary>
        /// Get exit code.
        /// </summary>
        [[nodiscard]] int GetExitCode() const noexcept;

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