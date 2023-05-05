#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>

#include <Module/Window.hpp>

#include <Log/Logger.hpp>

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
    { /*
         auto TextFile = std::make_shared<Asset::TextFileAsset>(
             L"Hello world");*/

        // auto p  = Asset::AssetHandle::Random();
        // auto px = boost::uuids::to_string(p);

        // Asset::AssetHandle Hndl(boost::uuids::string_generator()("6d9c45d1-657d-43ce-a0e2-6bb23add1755"));

        Asset::AssetResourceHandlers handlers;

        Asset::ZipAssetPack* z = new Asset::ZipAssetPack(handlers);

        m_AssetManager = std::make_shared<Asset::RuntimeResourceManager>();

        // auto Pack = m_AssetManager->NewPack<Asset::ZipAssetPack>("main_asset");
        // Pack->Save(
        //     p,
        //     TextFile);

        // Pack->Export("Test.np");

        auto Pack2 = m_AssetManager->LoadPack("teee", "Test.np");

        // auto Asset = Pack2->Load(Hndl);

        // auto Text = std::dynamic_pointer_cast<Asset::TextFileAsset>(Asset.lock());
        // NEON_INFO(Text->AsUtf8());

        // int x;
    }
} // namespace Neon
