#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>

#include <Resource/Runtime/Manager.hpp>
#include <Resource/Types/Logger.hpp>
#include <Resource/Pack.hpp>

#include <Module/Window.hpp>
#include <Module/Graphics.hpp>

#include <Runtime/Pipeline.hpp>
#include <Runtime/Phases/SplashScreen.hpp>

namespace Neon::Runtime
{
    DefaultGameEngine::DefaultGameEngine(
        const Config::EngineConfig& Config)
    {
        m_Window = std::make_unique<Module::Window>(this, Config);
    }

    DefaultGameEngine::~DefaultGameEngine()
    {
    }

    void DefaultGameEngine::Initialize()
    {
        auto ResourceManager = RegisterInterface<Asset::IResourceManager, Asset::RuntimeResourceManager>();

        //

        const auto LoggerAssetUid = Asset::AssetHandle::FromString("d0b50bba-f800-4c18-a595-fd5c4b380190");

        auto Pack = ResourceManager->LoadPack("__neon", "neonrt.np");

        // Set global logger settings
        {
            auto Logger = Pack->Load<Asset::LoggerAsset>(LoggerAssetUid);
            Logger->SetGlobal();
        }

        DispatchLoaderPipeline();
    }

    int DefaultGameEngine::Run()
    {
        Initialize();
        while (m_Window->Run())
        {
            m_Pipeline->BeginDispatch();
            m_Pipeline->EndDispatch();
        }
        Shutdown();
        return m_Window->GetExitCode();
    }

    Module::Window* DefaultGameEngine::GetWindowModule() noexcept
    {
        return m_Window.get();
    }

    Module::Graphics* DefaultGameEngine::GetGraphicsModule() noexcept
    {
        return m_Window->GetGraphics();
    }

    EnginePipeline& DefaultGameEngine::GetPipeline()
    {
        return *m_Pipeline;
    }

    void DefaultGameEngine::SetPipeline(
        UPtr<EnginePipeline> Pipeline)
    {
        m_Pipeline = std::move(Pipeline);
    }

    void DefaultGameEngine::DispatchLoaderPipeline()
    {
        EnginePipelineBuilder Builder;

        auto PreRender    = Builder.NewPhase("PreRender");
        auto SplashScreen = Builder.NewPhase("Render");
        auto PostRender   = Builder.NewPhase("PostRender");

        auto RHICompiler    = Builder.NewPhase("RHICompiler");
        auto ResourceLoader = Builder.NewPhase("ResourceLoader");

        //

        // Phases::SplashScreen::Build(Builder);

        //

        SplashScreen.DependsOn(PreRender);
        PostRender.DependsOn(SplashScreen);

        //

        SetPipeline(std::make_unique<EnginePipeline>(std::move(Builder)));
        m_Pipeline->SetThreadCount(4);

        m_Pipeline->Attach(
            "PreRender",
            [this]
            {
                auto Graphics = m_Window->GetGraphics();
                Graphics->PreRender();
            });

        m_Pipeline->Attach(
            "Render",
            [this]
            {
                auto Graphics = m_Window->GetGraphics();
                Graphics->Render();
            });

        m_Pipeline->Attach(
            "PostRender",
            [this]
            {
                auto Graphics = m_Window->GetGraphics();
                Graphics->PostRender();
            });

        Phases::SplashScreen::Bind(this);
    }
} // namespace Neon::Runtime
