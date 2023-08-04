#include <EnginePCH.hpp>
#include <Runtime/Types/WorldRuntime.hpp>

#include <Runtime/GameEngine.hpp>
#include <Runtime/Pipeline.hpp>

#include <Window/Window.hpp>
#include <RHI/Swapchain.hpp>

namespace Neon::Runtime
{
    EngineWorldRuntime::EngineWorldRuntime()
    {
        auto Engine = DefaultGameEngine::Get();

        EnginePipelineBuilder Builder;

        auto PreUpdate  = Builder.NewPhase("PreUpdate");
        auto Update     = Builder.NewPhase("Update");
        auto PostUpdate = Builder.NewPhase("PostUpdate");

        auto PreRender  = Builder.NewPhase("PreRender");
        auto Render     = Builder.NewPhase("Render");
        auto PostRender = Builder.NewPhase("PostRender");

        //

        PreUpdate.Then(Update).Then(PostUpdate);
        PreRender.Then(Render).Then(PostRender);

        //

        auto Pipeline = std::make_unique<EnginePipeline>(std::move(Builder));

        Pipeline->Attach(
            "Update",
            [this]
            {
                m_Scene.Update();
            });

        //

        Pipeline->Attach(
            "PreRender",
            [this]
            {
                m_IsRendering = DefaultGameEngine::Get()->GetWindow()->IsVisible().get();
                if (m_IsRendering)
                {
                    RHI::ISwapchain::Get()->PrepareFrame();
                }
            });

        Pipeline->Attach(
            "Render",
            [this]
            {
                if (m_IsRendering)
                {
                    m_Scene.Render();
                }
            });

        Pipeline->Attach(
            "PostRender",
            [this]
            {
                if (m_IsRendering)
                {
                    RHI::ISwapchain::Get()->Present(float(m_Scene.GetDeltaTime()));
                }
            });

        Engine->SetPipeline(std::move(Pipeline));
    }

    Scene::GameScene& EngineWorldRuntime::GetScene()
    {
        return m_Scene;
    }

    const Scene::GameScene& EngineWorldRuntime::GetScene() const
    {
        return m_Scene;
    }
} // namespace Neon::Runtime