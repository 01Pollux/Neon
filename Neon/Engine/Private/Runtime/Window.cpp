#include <EnginePCH.hpp>
#include <Runtime/Window.hpp>
#include <Runtime/GameEngine.hpp>

namespace Neon::Runtime
{
    EngineWindow::EngineWindow(
        Runtime::DefaultGameEngine* Engine,
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
    }

    int EngineWindow::GetExitCode() const noexcept
    {
        return m_ExitCode;
    }

    Windowing::IWindowApp* EngineWindow::GetWindow() const noexcept
    {
        return m_Window.get();
    }

    bool EngineWindow::Run()
    {
        namespace WinEvents = Windowing::Events;

        Windowing::Event Msg;
        while (m_Window->PeekEvent(Msg))
        {
            std::visit(
                VariantVisitor{
                    [this](const WinEvents::SizeChanged& SizeMsg)
                    {
                        OnWindowSizeChanged().Broadcast(SizeMsg.NewSize);
                    } },
                Msg);
        }
        return m_Window->IsRunning();
    }
} // namespace Neon::Runtime