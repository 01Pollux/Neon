#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>

#include <Module/Window.hpp>

#include <Log/Logger.hpp>

#include <Resource/Runtime/Manager.hpp>

#include <Resource/Types/TextFile.hpp>
#include <Resource/Packs/ZipPack.hpp>

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
        auto TextFile = std::make_shared<Asset::TextFileAsset>(
            L"Hello world");

        m_AssetManager = std::make_shared<Asset::RuntimeResourceManager>();

        auto Pack = m_AssetManager->NewPack<Asset::ZipAssetPack>("main_asset");
        Pack->Save(
            Asset::AssetHandle::Random(),
            TextFile);

        Pack->Export(R"(D:\Dev\Engines\Neon\test.np)");
    }
} // namespace Neon
