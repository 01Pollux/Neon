#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>
#include <Runtime/GameLogic.hpp>
#include <Runtime/DebugOverlay.hpp>
#include <Script/Engine.hpp>

//

#include <Asset/Manager.hpp>
#include <Asset/Storage.hpp>

#include <Asset/Handlers/Json.hpp>
#include <Asset/Handlers/Logger.hpp>
#include <Asset/Handlers/Model.hpp>
#include <Asset/Handlers/PropertyTree.hpp>
#include <Asset/Handlers/RootSignature.hpp>
#include <Asset/Handlers/RuntimeScene.hpp>
#include <Asset/Handlers/Shader.hpp>
#include <Asset/Handlers/Texture.hpp>
#include <Asset/Handlers/TextFile.hpp>

//

#include <Window/Window.hpp>
#include <RHI/Device.hpp>
#include <RHI/Swapchain.hpp>
#include <RHI/ImGui.hpp>
#include <RHI/Material/Shared.hpp>

//

#include <Log/Logger.hpp>

namespace Neon::Runtime
{
    static GameEngine* s_GameEngine = nullptr;

    GameEngine::GameEngine()
    {
        NEON_ASSERT(!s_GameEngine);
        s_GameEngine = this;

        // Initialize the asset system
        Asset::Storage::Initialize();
    }

    GameEngine::~GameEngine()
    {
        // Unregister the scripting system
        Scripting::Shutdown();

        // Shutdown the asset system
        Asset::Storage::Shutdown();

        // Shutdown the game logic
        m_Logic.reset();

        // Shutdown the debug overlay
        Runtime::DebugOverlay::Destroy();

        // Shutdown the entity world
        Scene::EntityWorld::Shutdown();

        // Shutdown the shared materials
        RHI::SharedMaterials::Shutdown();

        // Shutdown the renderer
        RHI::IRenderDevice::Destroy();

        // Shutdown the window
        m_Window.reset();

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
        // Register the scripting system
        Scripting::Initialize(Config.Script);

        // Initialize and load assets packages
        InitializeAssetSystem(Config);

        // Initialize the window
        m_Window = std::make_unique<Windowing::WindowApp>(Config.Window);

        // Initialize the renderer
        auto& RendererConfig = Config.Renderer;
        RHI::IRenderDevice::Create(m_Window.get(), RendererConfig.Device, RendererConfig.Swapchain);

        //  Initialize the debug overlay
        Runtime::DebugOverlay::Create();

        // Initialize the shared materials
        RHI::SharedMaterials::Initialize();

        // Initialize the game logic
        m_Logic = std::make_unique<GameLogic>();
    }

    void GameEngine::Run()
    {
        bool IsMinimized = m_Window->IsMinimized();

        m_Window->OnWindowMinized()
            .Listen([&IsMinimized](bool Minimized)
                    { IsMinimized = Minimized; });

        std::optional<Size2I> LastSize;
        // Listen for resize events
        m_Window->OnWindowSizeChanged()
            .Listen([&LastSize](const Size2I& Size)
                    { LastSize = Size; });

        // Reset the timer before entering the loop to avoid a large delta time on the first frame
        m_GameTimer.Reset();
        while (m_Window->IsRunning())
        {
            m_Window->ProcessEvents();
            if (m_GameTimer.Tick())
            {
                Runtime::DebugOverlay::Reset();

                PreUpdate();
                m_Logic->Update();
                PostUpdate();

                if (!IsMinimized)
                {
                    if (LastSize)
                    {
                        RHI::ISwapchain::Get()->Resize(*LastSize);
                        LastSize.reset();
                    }

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
    }

    //

    Windowing::WindowApp* GameEngine::GetWindow() const
    {
        return m_Window.get();
    }

    GLFWwindow* GameEngine::GetWindowHandle() const
    {
        return m_Window->GetHandle();
    }

    GameLogic* GameEngine::GetLogic() const noexcept
    {
        return m_Logic.get();
    }

    Asio::ThreadPool<>& GameEngine::GetThreadPool() noexcept
    {
        return m_ThreadPool;
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
        Asset::Storage::RegisterHandler<Asset::JsonAsset::Handler>();
        Asset::Storage::RegisterHandler<Asset::LoggerAsset::Handler>();
        Asset::Storage::RegisterHandler<Asset::ModelAsset::Handler>();
        Asset::Storage::RegisterHandler<Asset::PropertyTreeAsset::Handler>();
        Asset::Storage::RegisterHandler<Asset::RootSignatureAsset::Handler>();
        Asset::Storage::RegisterHandler<Asset::RuntimeSceneAsset::Handler>();
        Asset::Storage::RegisterHandler<Asset::ShaderAsset::Handler>();
        Asset::Storage::RegisterHandler<Asset::TextFileAsset::Handler>();
        Asset::Storage::RegisterHandler<Asset::TextureAsset::Handler>();

        //

        LoadPacks(Config);
        if (Config.Resource.LoggerAssetUid)
        {
            // Set global logger settings
            if (auto Logger = std::dynamic_pointer_cast<Asset::LoggerAsset>(Asset::Manager::Load(*Config.Resource.LoggerAssetUid)))
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
