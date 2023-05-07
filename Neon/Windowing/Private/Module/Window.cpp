#include <WindowPCH.hpp>
#include <Module/Window.hpp>

namespace Neon::Module
{
    Window::Window(
        Neon::World&                   World,
        const String&                  Title,
        const Size2I&                  Size,
        const Windowing::MWindowStyle& Style)
    {
        m_Window.reset(Windowing::IWindowApp::Create(Title, Size, Style));

        World->system("Window::MessageLoop")
            .kind(flecs::PreFrame)
            .iter([this](flecs::iter& Iter)
                  { MessageLoop(Iter); });
    }

    int Window::GetExitCode() const noexcept
    {
        return m_ExitCode;
    }

    void Window::MessageLoop(
        flecs::iter& Iter)
    {
        Windowing::Event Msg;

        while (m_Window->PeekEvent(&Msg))
        {
            if (auto Close = std::get_if<Windowing::Events::Close>(&Msg))
            {
                m_ExitCode = Close->ExitCode;
                Iter.world().quit();
            }
        }
    }
} // namespace Neon::Module