#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>

#include <Resource/Runtime/Manager.hpp>
#include <Resource/Types/Logger.hpp>
#include <Resource/Pack.hpp>

#include <Runtime/Window.hpp>
#include <Runtime/Renderer.hpp>
#include <Runtime/Pipeline.hpp>

// #include <Runtime/Types/LoadingScene.hpp>
#include <Runtime/Types/WorldRuntime.hpp>

namespace Neon::Runtime
{
    void DefaultGameEngine::Initialize(
        const Config::EngineConfig& Config)
    {
        auto ResourceManager = RegisterInterface<Asset::IAssetManager, Asset::RuntimeAssetManager>();

        const auto LoggerAssetUid = Asset::AssetHandle::FromString("d0b50bba-f800-4c18-a595-fd5c4b380190");

        auto Pack = ResourceManager->LoadPack("__neon", "neonrt.np");
        // Set global logger settings
        if (auto Logger = Pack->Load<Asset::LoggerAsset>(LoggerAssetUid))
        {
            Logger->SetGlobal();
        }

        //

        LoadPacks(Config);

        //

        RegisterInterface<EngineWindow>(this, Config);
        RegisterInterface<EngineRenderer>(this, Config);

        // RegisterInterface<EngineRuntime, LoadingScreenRuntime>(this);
        RegisterInterface<EngineRuntime, EngineWorldRuntime>(this);
    }

    int DefaultGameEngine::Run()
    {
        auto Window = QueryInterface<EngineWindow>();
        while (Window->Run())
        {
            if (auto Pipeline = QueryInterface<EnginePipeline>())
            {
                Pipeline->BeginDispatch();
                Pipeline->EndDispatch();
            }
        }
        Shutdown();
        return Window->GetExitCode();
    }

    void DefaultGameEngine::LoadPacks(
        const Config::EngineConfig& Config)
    {
        if (auto ResourceManager = QueryInterface<Asset::IAssetManager>())
        {
            for (auto& [PackName, Path] : Config.Resource.Packs)
            {
                ResourceManager->LoadPack(PackName, Path);
            }
        }
    }
} // namespace Neon::Runtime
