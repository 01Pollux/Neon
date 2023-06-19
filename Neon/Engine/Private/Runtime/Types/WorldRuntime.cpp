#include <EnginePCH.hpp>
#include <Runtime/Types/WorldRuntime.hpp>

#include <Runtime/GameEngine.hpp>
#include <Runtime/Renderer.hpp>
#include <Runtime/Pipeline.hpp>

namespace Neon::Runtime
{
    EngineWorldRuntime::EngineWorldRuntime(
        DefaultGameEngine* Engine)
    {
        EnginePipelineBuilder Builder;

        auto PreUpdate  = Builder.NewPhase("PreUpdate");
        auto Update     = Builder.NewPhase("Update");
        auto PostUpdate = Builder.NewPhase("PostUpdate");

        auto PreRender  = Builder.NewPhase("PreRender");
        auto Render     = Builder.NewPhase("Render");
        auto PostRender = Builder.NewPhase("PostRender");

        //

        Update.DependsOn(PreUpdate);
        PostUpdate.DependsOn(Update);

        PreRender.DependsOn(PostUpdate);
        Render.DependsOn(PreRender);

        //

        auto Pipeline = Engine->OverwriteInterface<EnginePipeline>(std::move(Builder), 2);
        auto Renderer = Engine->QueryInterface<EngineRenderer>();

        //

        Pipeline->Attach(
            "Update",
            [this]
            {
                m_Scene->progress();
            });

        //

        Pipeline->Attach(
            "PreRender",
            [this, Renderer]
            {
                Renderer->PreRender();
            });

        Pipeline->Attach(
            "Render",
            [this, Renderer]
            {
                Renderer->Render();
            });

        Pipeline->Attach(
            "PostRender",
            [this, Renderer]
            {
                Renderer->PostRender();
            });

        m_Scene.Test();
        SetupRenderPasses(Renderer.get());
    }

    Scene::GameScene& EngineWorldRuntime::GetScene()
    {
        return m_Scene;
    }

    const Scene::GameScene& EngineWorldRuntime::GetScene() const
    {
        return m_Scene;
    }

    void EngineWorldRuntime::SetupRenderPasses(
        EngineRenderer* Renderer)
    {
        auto RenderGraph = Renderer->GetRenderGraph();
        auto Builder     = RenderGraph->Reset();

        Builder.Build();
    }
} // namespace Neon::Runtime