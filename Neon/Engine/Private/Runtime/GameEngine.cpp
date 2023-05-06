#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>

#include <Module/Window.hpp>

#include <Resource/Runtime/Manager.hpp>

#include <Resource/Types/TextFile.hpp>
#include <Resource/Packs/ZipPack.hpp>

#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/string_generator.hpp>

//

#include <Resource/Pack.hpp>
#include <Resource/Handler.hpp>

namespace Neon
{
    DefaultGameEngine::DefaultGameEngine(
        const Config::EngineConfig& Config)
    {
        CreateWindow(Config.Window);
        LoadResourcePacks();
    }

    DefaultGameEngine::~DefaultGameEngine()
    {
    }

    int DefaultGameEngine::Run()
    {
        while (!m_World->should_quit())
        {
            m_World->progress();
        }
        return m_World.Module<Module::Window>()->GetExitCode();
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
