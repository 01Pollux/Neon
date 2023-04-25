#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>

namespace Neon
{
    DefaultGameEngine::DefaultGameEngine(
        const Config::EngineConfig& Config)
    {
        CreateWindow(Config.Window);
    }

    DefaultGameEngine::~DefaultGameEngine()
    {
    }

    int DefaultGameEngine::Run()
    {
        std::optional<int> ExitCode;
        Windowing::Event   Msg;

        while (!ExitCode)
        {
            while (m_Window->PeekEvent(&Msg))
            {
                if (auto Close = std::get_if<Windowing::Events::Close>(&Msg))
                {
                    ExitCode = Close->ExitCode;
                }
                else if (auto SizeChanged = std::get_if<Windowing::Events::SizeChanged>(&Msg))
                {
                    printf("%i,%i\n", SizeChanged->NewSize.Width(), SizeChanged->NewSize.Height());
                }
            }
        }

        return *ExitCode;
    }

    //

    void DefaultGameEngine::CreateWindow(
        const Config::WindowConfig& Config)
    {
        Windowing::MWindowStyle Style;
        if (Config.WithCloseButton)
        {
            Style.Set(Windowing::EWindowStyle::Close);
        }
        if (Config.CanResize)
        {
            Style.Set(Windowing::EWindowStyle::Resize);
        }
        if (Config.HasTitleBar)
        {
            Style.Set(Windowing::EWindowStyle::TitleBar);
        }
        if (Config.Windowed)
        {
            Style.Set(Windowing::EWindowStyle::Windowed);
        }
        if (Config.Fullscreen)
        {
            Style.Set(Windowing::EWindowStyle::Fullscreen);
        }
        m_Window.reset(Windowing::IWindowApp::Create(Config.Title, Config.Size, Style));
    }
} // namespace Neon
