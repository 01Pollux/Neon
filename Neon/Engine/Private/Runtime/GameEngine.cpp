#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>
#include <Runtime/DebugOverlay.hpp>

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

    DefaultGameEngine::DefaultGameEngine() :
        m_Scene(std::make_unique<Scene::GameScene>()),
        m_ThreadPool(std::make_unique<Asio::ThreadPool<>>(4))
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

        Runtime::DebugOverlay::Create();
    }

    void DefaultGameEngine::Shutdown()
    {
        Runtime::DebugOverlay::Destroy();
    }

    int DefaultGameEngine::Run()
    {
        // Reset the timer before entering the loop to avoid a large delta time on the first frame
        m_GameTimer.Reset();
        while (m_Window->Run())
        {
            m_Window->ProcessInputs();
            if (m_GameTimer.Tick())
            {
                Runtime::DebugOverlay::Reset();
                auto IsScreenVisible = GetWindow()->IsVisible();

                m_Scene->Update();
                if (IsScreenVisible.get())
                {
                    RHI::ISwapchain::Get()->PrepareFrame();
                    m_Scene->Render();
                    RHI::ISwapchain::Get()->Present(float(m_GameTimer.GetDeltaTime()));
                }
            }
        }
        Shutdown();
        return m_Window->GetExitCode();
    }

    //

    Windowing::IWindowApp* DefaultGameEngine::GetWindow() const
    {
        return m_Window->GetWindow();
    }

    Scene::GameScene& DefaultGameEngine::GetScene() const noexcept
    {
        return *m_Scene;
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

    //

    double DefaultGameEngine::GetGameTime() const
    {
        return m_GameTimer.GetGameTime();
    }

    double DefaultGameEngine::GetEngineTime() const
    {
        return m_GameTimer.GetEngineTime();
    }

    double DefaultGameEngine::GetDeltaTime() const
    {
        return m_GameTimer.GetDeltaTime();
    }

    float DefaultGameEngine::GetTimeScale() const
    {
        return m_GameTimer.GetTimeScale();
    }

    void DefaultGameEngine::SetTimeScale(
        float TimeScale)
    {
        m_GameTimer.SetTimeScale(TimeScale);
    }
} // namespace Neon::Runtime
