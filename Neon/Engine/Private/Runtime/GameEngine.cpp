#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>

#include <Asset/Types/Logger.hpp>
#include <Asset/Manager.hpp>
#include <Asset/Storage.hpp>

#include <cppcoro/sync_wait.hpp>

#include <Log/Logger.hpp>

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
        Config::EngineConfig Config)
    {
        LoadPacks(Config);
        if (Config.Resource.LoggerAssetUid)
        {
            // Set global logger settings
            if (auto Logger = Asset::AssetTaskPtr<Asset::LoggerAsset>(Asset::Manager::Load(*Config.Resource.LoggerAssetUid)))
            {
                Logger->SetGlobal();
            }
        }

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
        Config::EngineConfig& Config)
    {
        for (auto& Pack : Config.Resource.AssetPackages)
        {
            Asset::Storage::Mount(std::move(Pack));
        }
    }
} // namespace Neon::Runtime
