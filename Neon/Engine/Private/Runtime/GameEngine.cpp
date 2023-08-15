#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>
#include <Runtime/GameLogic.hpp>
#include <Runtime/DebugOverlay.hpp>

#include <Asset/Manager.hpp>
#include <Asset/Storage.hpp>

#include <Asset/Types/Logger.hpp>
#include <Asset/Handlers/TextFile.hpp>
#include <Asset/Handlers/Texture.hpp>
#include <Asset/Handlers/Logger.hpp>
#include <Asset/Handlers/RootSignature.hpp>
#include <Asset/Handlers/Shader.hpp>

#include <RHI/Swapchain.hpp>
#include <RHI/ImGui.hpp>

#include <Log/Logger.hpp>

namespace Neon::Runtime
{
    static GameEngine* s_GameEngine = nullptr;

    GameEngine::GameEngine() :
        m_Logic(std::make_unique<GameLogic>()),
        m_ThreadPool(std::make_unique<Asio::ThreadPool<>>(4))
    {
        NEON_ASSERT(!s_GameEngine);
        s_GameEngine = this;
    }

    GameEngine::~GameEngine()
    {
        NEON_ASSERT(s_GameEngine);
        s_GameEngine = nullptr;
    }

    GameEngine* GameEngine::Get()
    {
        return s_GameEngine;
    }

    void GameEngine::Initialize(
        Config::EngineConfig Config)
    {
        InitializeAssetSystem(Config);

        //

        m_Window.reset(NEON_NEW EngineWindow(Config));

        Runtime::DebugOverlay::Create();
    }

    void GameEngine::Shutdown()
    {
        Runtime::DebugOverlay::Destroy();
    }

    void GameEngine::PostRender()
    {
    }

    int GameEngine::Run()
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

                PreUpdate();
                m_Logic->Update();
                PostUpdate();

                if (IsScreenVisible.get())
                {
                    RHI::ISwapchain::Get()->PrepareFrame();

                    RHI::ImGuiRHI::BeginImGuiFrame();

                    PreRender();
                    m_Logic->Render();
                    PostRender();

                    RHI::ImGuiRHI::EndImGuiFrame();

                    RHI::ISwapchain::Get()->Present(float(m_GameTimer.GetDeltaTime()));
                }
            }
        }
        Shutdown();
        return m_Window->GetExitCode();
    }

    //

    Windowing::IWindowApp* GameEngine::GetWindow() const
    {
        return m_Window->GetWindow();
    }

    GameLogic* GameEngine::GetLogic() const noexcept
    {
        return m_Logic.get();
    }

    void GameEngine::LoadPacks(
        Config::EngineConfig& Config)
    {
        for (auto& Pack : Config.Resource.AssetPackages)
        {
            Asset::Storage::Mount(std::move(Pack));
        }
    }

    void GameEngine::InitializeAssetSystem(
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

    double GameEngine::GetGameTime() const
    {
        return m_GameTimer.GetGameTime();
    }

    double GameEngine::GetEngineTime() const
    {
        return m_GameTimer.GetEngineTime();
    }

    double GameEngine::GetDeltaTime() const
    {
        return m_GameTimer.GetDeltaTime();
    }

    float GameEngine::GetTimeScale() const
    {
        return m_GameTimer.GetTimeScale();
    }

    void GameEngine::SetTimeScale(
        float TimeScale)
    {
        m_GameTimer.SetTimeScale(TimeScale);
    }
} // namespace Neon::Runtime
