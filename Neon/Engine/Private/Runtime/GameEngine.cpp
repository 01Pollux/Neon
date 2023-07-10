#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>

#include <Resource/Runtime/Manager.hpp>
#include <Resource/Types/Logger.hpp>
#include <Resource/Pack.hpp>

#include <cppcoro/sync_wait.hpp>

namespace Neon::Runtime
{
    static DefaultGameEngine* s_GameEngine = nullptr;

    DefaultGameEngine::DefaultGameEngine()
    {
        NEON_ASSERT(!s_GameEngine);
        s_GameEngine = this;
    }

    DefaultGameEngine::~DefaultGameEngine()
    {
        NEON_ASSERT(s_GameEngine);
        s_GameEngine = nullptr;
    }

    DefaultGameEngine* DefaultGameEngine::Get()
    {
        return s_GameEngine;
    }

    void DefaultGameEngine::Initialize(
        const Config::EngineConfig& Config)
    {
        auto ResourceManager = RegisterInterface<Asset::IAssetManager, Asset::RuntimeAssetManager>();
        if (Config.LoggerAssetUid)
        {
            auto Pack = ResourceManager->LoadPack("__neon", "../../../Runtimes/neonrt.np");
            // Set global logger settings
            if (auto Logger = Pack->Load<Asset::LoggerAsset>(*Config.LoggerAssetUid))
            {
                Logger->SetGlobal();
            }
        }

        LoadPacks(Config);

        //

        m_Window.reset(NEON_NEW EngineWindow(Config));
    }

    int DefaultGameEngine::Run()
    {
        while (m_Window->Run())
        {
            if (m_PendingPipeline)
            {
                m_Pipeline = std::move(m_PendingPipeline.value());
                m_PendingPipeline.reset();
            }
            if (auto Pipeline = m_Pipeline.get())
            {
                cppcoro::sync_wait(Pipeline->Dispatch());
            }
        }
        Shutdown();
        UnregisterAllInterfaces();
        return m_Window->GetExitCode();
    }

    Windowing::IWindowApp* DefaultGameEngine::GetWindow() const
    {
        return m_Window->GetWindow();
    }

    EnginePipeline* DefaultGameEngine::GetPipeline() const
    {
        return m_Pipeline.get();
    }

    void DefaultGameEngine::SetPipeline(
        UPtr<EnginePipeline> Pipeline)
    {
        m_PendingPipeline = std::move(Pipeline);
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
