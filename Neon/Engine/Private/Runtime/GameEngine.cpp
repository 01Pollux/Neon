#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>

#include <Asset/Manager.hpp>
#include <Asset/Storage.hpp>

#include <Asset/Types/Logger.hpp>
#include <Asset/Handlers/TextFile.hpp>
#include <Asset/Handlers/Texture.hpp>
#include <Asset/Handlers/Logger.hpp>
#include <Asset/Handlers/RootSignature.hpp>
#include <Asset/Handlers/Shader.hpp>

#include <Input/System.hpp>

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
        InitializeAssetSystem(Config);

        //

        m_Window.reset(NEON_NEW EngineWindow(Config));
    }

    int DefaultGameEngine::Run()
    {
        while (m_Window->Run())
        {
            m_Window->ProcessInputs();
            if (m_PendingPipeline)
            {
                m_Pipeline = std::move(m_PendingPipeline.value());
                m_PendingPipeline.reset();
            }
            if (auto Pipeline = m_Pipeline.get())
            {
                Pipeline->Dispatch();
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
        UPtr<EnginePipeline> Pipeline,
        bool                 Immediate)
    {
        if (Immediate)
        {
            m_Pipeline = std::move(Pipeline);
        }
        else
        {
            m_PendingPipeline = std::move(Pipeline);
        }
    }

    void DefaultGameEngine::LoadPacks(
        Config::EngineConfig& Config)
    {
        for (auto& Pack : Config.Resource.AssetPackages)
        {
            Asset::Storage::Mount(std::move(Pack));
        }
    }

    void DefaultGameEngine::InitializeAssetSystem(
        Config::EngineConfig& Config)
    {
        Asset::Storage::RegisterHandler<Asset::TextFileAsset::Handler>();
        Asset::Storage::RegisterHandler<Asset::TextureAsset::Handler>();
        Asset::Storage::RegisterHandler<Asset::LoggerAsset::Handler>();
        Asset::Storage::RegisterHandler<Asset::RootSignatureAsset::Handler>();
        Asset::Storage::RegisterHandler<Asset::ShaderAsset::Handler>();

        //

        LoadPacks(Config);
        if (Config.Resource.LoggerAssetUid)
        {
            // Set global logger settings
            if (auto Logger = Asset::AssetTaskPtr<Asset::LoggerAsset>(Asset::Manager::Load(*Config.Resource.LoggerAssetUid)))
            {
                Logger->SetGlobal();
            }
        }
    }
} // namespace Neon::Runtime
