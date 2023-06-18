#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>

#include <Resource/Runtime/Manager.hpp>
#include <Resource/Types/Logger.hpp>
#include <Resource/Pack.hpp>

#include <Runtime/Window.hpp>
#include <Runtime/Renderer.hpp>

#include <Runtime/Pipeline.hpp>
#include <Runtime/Phases/SplashScreen.hpp>

namespace Neon::Runtime
{
    DefaultGameEngine::~DefaultGameEngine()
    {
    }

    void DefaultGameEngine::Initialize(
        const Config::EngineConfig& Config)
    {
        auto ResourceManager = RegisterInterface<Asset::IResourceManager, Asset::RuntimeResourceManager>();

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

        RegisterInterface<EnginetWindow>(this, Config);
        RegisterInterface<EngineRenderer>(this, Config);

        RegisterSplashScreenPipeline();
    }

    int DefaultGameEngine::Run()
    {
        auto Window = QueryInterface<EnginetWindow>();
        while (Window->Run())
        {
            auto Pipeline = QueryInterface<EnginePipeline>();

            if (Pipeline)
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
        if (auto ResourceManager = QueryInterface<Asset::IResourceManager>())
        {
            for (auto& [PackName, Path] : Config.Resource.Packs)
            {
                ResourceManager->LoadPack(PackName, Path);
            }
        }
    }

    void DefaultGameEngine::RegisterSplashScreenPipeline()
    {
        EnginePipelineBuilder Builder;

        auto Render         = Builder.NewPhase("Render");
        auto ResourceLoader = Builder.NewPhase("ResourceLoader");

        //

        auto Pipeline = RegisterInterface<EnginePipeline>(std::move(Builder), 2);
        auto Renderer = QueryInterface<EngineRenderer>();

        Pipeline->Attach(
            "Render",
            [this, Renderer]
            {
                Renderer->PreRender();
                Renderer->Render();
                Renderer->PostRender();
            });

        Phases::SplashScreen::Bind(this);
    }
} // namespace Neon::Runtime
