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
        while (true)
        {
        }
        return 0;
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
        if (Config.StartFullScreen)
        {
            Style.Set(Windowing::EWindowStyle::Fullscreen);
        }
        m_Window.reset(Windowing::IWindowApp::Create(Config.Title, Config.Size, Style));
    }
} // namespace Neon
