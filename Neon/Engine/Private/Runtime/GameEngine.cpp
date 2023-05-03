#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>

#include <Module/Window.hpp>

#include <Log/Logger.hpp>

#include <Resource/Runtime/Manager.hpp>

namespace Neon
{
    DefaultGameEngine::DefaultGameEngine(
        const Config::EngineConfig& Config)
    {
        Logger::Initialize();
        CreateWindow(Config.Window);
        LoadResourcePacks();
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

    Asset::IResourceManager* DefaultGameEngine::GetResourceManager()
    {
        return m_AssetManager.get();
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

    void DefaultGameEngine::LoadResourcePacks()
    {
        m_AssetManager = std::make_shared<Asset::RuntimeResourceManager>();
    }
} // namespace Neon
