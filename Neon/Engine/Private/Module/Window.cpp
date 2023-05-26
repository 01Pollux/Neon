#include <EnginePCH.hpp>
#include <Module/Window.hpp>
#include <Module/Graphics.hpp>

namespace Neon::Module
{
    Window::Window(
        Neon::World&                World,
        const Config::EngineConfig& Config)
    {
        World.ModuleScope<Window>();

        {
            auto& WindowConfig = Config.Window;

            Windowing::MWindowStyle Style;
            if (WindowConfig.WithCloseButton)
            {
                Style.Set(Windowing::EWindowStyle::Close);
            }
            if (WindowConfig.CanResize)
            {
                Style.Set(Windowing::EWindowStyle::Resize);
            }
            if (WindowConfig.HasTitleBar)
            {
                Style.Set(Windowing::EWindowStyle::TitleBar);
            }
            if (WindowConfig.Windowed)
            {
                Style.Set(Windowing::EWindowStyle::Windowed);
            }
            if (WindowConfig.Fullscreen)
            {
                Style.Set(Windowing::EWindowStyle::Fullscreen);
            }
            m_Window.reset(Windowing::IWindowApp::Create(WindowConfig.Title, WindowConfig.Size, Style, WindowConfig.StartInMiddle));
        }

        World->system("Window::MessageLoop")
            .kind(flecs::PreFrame)
            .iter([this](flecs::iter& Iter)
                  { MessageLoop(Iter); });

        World.Import<Graphics>(Config, this);
    }

    int Window::GetExitCode() const noexcept
    {
        return m_ExitCode;
    }

    Windowing::IWindowApp* Window::GetWindow() const noexcept
    {
        return m_Window.get();
    }

    void Window::MessageLoop(
        flecs::iter& Iter)
    {
        namespace WinEvents = Windowing::Events;

        Windowing::Event Msg;

        while (m_Window->PeekEvent(&Msg))
        {
            std::visit(
                VariantVisitor{
                    [this, &Iter](const WinEvents::Close& CloseMsg)
                    {
                        m_ExitCode = CloseMsg.ExitCode;
                        Iter.world().quit();
                    },
                    [this, &Iter](const WinEvents::SizeChanged& SizeMsg)
                    {
                        OnWindowSizeChanged().Broadcast(SizeMsg.NewSize);
                    } },
                Msg);
        }
    }
} // namespace Neon::Module