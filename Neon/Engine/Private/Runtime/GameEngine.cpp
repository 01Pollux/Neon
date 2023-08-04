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

#include <Scene/Scene.hpp>
#include <RHI/Swapchain.hpp>

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
        SetScene(std::move(Config.FirstScene));
    }

    int DefaultGameEngine::Run()
    {
        while (m_Window->Run())
        {
            m_Window->ProcessInputs();
            if (m_Scene)
            {
                auto IsScreenVisible = GetWindow()->IsVisible();

                m_Scene->Update();

                if (IsScreenVisible.get())
                {
                    RHI::ISwapchain::Get()->PrepareFrame();
                    m_Scene->Render();
                    RHI::ISwapchain::Get()->Present(float(m_Scene->GetDeltaTime()));
                }
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

    Scene::GameScene& DefaultGameEngine::GetScene() const noexcept
    {
        NEON_ASSERT(m_Scene);
        return *m_Scene;
    }

    void DefaultGameEngine::SetScene(
        UPtr<Scene::GameScene> Scene)
    {
        m_Scene = std::move(Scene);
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
