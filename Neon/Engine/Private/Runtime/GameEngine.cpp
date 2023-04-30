#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>

#include <Module/Window.hpp>

#include <Log/Logger.hpp>

#include <Resource/Handle.hpp>

#include <boost/chrono.hpp>

namespace Neon
{
    DefaultGameEngine::DefaultGameEngine(
        const Config::EngineConfig& Config)
    {
        boost::chrono::system_clock::time_point start = boost::chrono::system_clock::now();

        for (long i = 0; i < 10000000; ++i)
            (void)std::sqrt(123.456L); // burn some time

        boost::chrono::duration<double> sec = boost::chrono::system_clock::now() - start;

        Logger::Initialize();
        CreateWindow(Config.Window);
    }

    DefaultGameEngine::~DefaultGameEngine()
    {
        Logger::Shutdown();
    }

    int DefaultGameEngine::Run()
    {
        while (!m_World->should_quit())
        {
            m_World->progress();
        }
        return m_World->get<Module::Window>()->GetExitCode();
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
        m_World.Import<Module::Window>(Config.Title, Config.Size, Style);
    }
} // namespace Neon
