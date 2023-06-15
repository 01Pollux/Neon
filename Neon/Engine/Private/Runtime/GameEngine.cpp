#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>

#include <Module/Window.hpp>
#include <Module/Resource.hpp>
#include <Module/Graphics.hpp>

#include <Runtime/Pipeline.hpp>

namespace Neon::Runtime
{
    DefaultGameEngine::DefaultGameEngine(
        const Config::EngineConfig& Config)
    {
        LoadResourcePacks(Config.Resource);
        m_Window = std::make_unique<Module::Window>(this, Config);
        DispatchLoaderPipeline();
    }

    DefaultGameEngine::~DefaultGameEngine()
    {
    }

    int DefaultGameEngine::Run()
    {
        Initialize();
        auto Graphics = m_Window->GetGraphics();
        while (m_Window->Run())
        {
            m_Pipeline->Dispatch();
        }
        Shutdown();
        return m_Window->GetExitCode();
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

    void DefaultGameEngine::LoadResourcePacks(
        const Config::ResourceConfig& Config)
    {
        m_ResourceManager = std::make_unique<Module::ResourceManager>(this, Config.Manager);
        for (auto& [Tag, Path] : Config.Packs)
        {
            m_ResourceManager->Get()->TryLoadPack(Tag, Path);
        }
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
            });

        m_Pipeline->Attach(
            "Render",
            [this]
            {
                auto Graphics = m_Window->GetGraphics();
            });

        m_Pipeline->Attach(
            "Render",
            [this]
            {
                auto Graphics = m_Window->GetGraphics();
            });

        m_Pipeline->Attach(
            "RHICompiler",
            [this]
            {
                auto Graphics = m_Window->GetGraphics();
            });

        m_Pipeline->Attach(
            "ResourceLoader",
            [this]
            {
                auto Graphics = m_Window->GetGraphics();
            });

        m_Pipeline->Attach(
            "PostRender",
            [this]
            {
                auto Graphics = m_Window->GetGraphics();
                Graphics->PostRender();
            });
    }
} // namespace Neon::Runtime
