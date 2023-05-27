#include <EnginePCH.hpp>
#include <Module/Window.hpp>
#include <Module/Graphics.hpp>

namespace Neon::Module
{
    Window::Window(
        DefaultGameEngine*          Engine,
        const Config::EngineConfig& Config)
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

        m_Graphics = std::make_unique<Module::Graphics>(Engine, Config, this);
    }

    Window::~Window()
    {
    }

    int Window::GetExitCode() const noexcept
    {
        return m_ExitCode;
    }

    Windowing::IWindowApp* Window::GetWindow() const noexcept
    {
        return m_Window.get();
    }

    Graphics* Window::GetGraphics() const noexcept
    {
        return m_Graphics.get();
    }

    bool Window::Run()
    {
        namespace WinEvents = Windowing::Events;

        Windowing::Event Msg;

        bool keepRunning = true;
        while (m_Window->PeekEvent(&Msg))
        {
            std::visit(
                VariantVisitor{
                    [this, &keepRunning](const WinEvents::Close& CloseMsg)
                    {
                        m_ExitCode  = CloseMsg.ExitCode;
                        keepRunning = false;
                    },
                    [this](const WinEvents::SizeChanged& SizeMsg)
                    {
                        OnWindowSizeChanged().Broadcast(SizeMsg.NewSize);
                    } },
                Msg);
        }
        return keepRunning;
    }
} // namespace Neon::Module