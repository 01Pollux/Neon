#include <EnginePCH.hpp>
#include <Runtime/Types/WorldRuntime.hpp>

#include <Resource/Manager.hpp>
#include <Resource/Pack.hpp>

#include <Runtime/GameEngine.hpp>
#include <Runtime/Renderer.hpp>
#include <Runtime/Pipeline.hpp>

//

#include <Renderer/RG/Passes/ScenePass.hpp>

//

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Sprite.hpp>

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
        auto MainPack = Engine->QueryInterface<Asset::IAssetManager>()->GetPack("__neon");

        auto DefaultShaderLib = Asset::AssetHandle::FromString("7427990f-9be1-4a23-aad5-1b99f00c29fd");
        auto ShaderLib        = MainPack->Load<Asset::ShaderLibraryAsset>(DefaultShaderLib);

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

        SetupRenderPasses(Renderer.get(), ShaderLib);

        //

        for (float y = -1.f; y < 1.f; y += .2f)
        {
            for (float x = -1.f; x < 1.f; x += .2f)
            {
                constexpr float Size = 0.2f;

                auto Sprite = m_Scene->entity();

                Scene::Component::Transform Transform;
                Transform.Local.SetPosition(Vector3(x, y, 0.f));
                Transform.World.SetPosition(Vector3(x, y, 0.f));
                Sprite.set(Transform);

                Scene::Component::Sprite SpriteComponent;

                SpriteComponent.ModulationColor =
                    Color4(0.0f, 1.0f, 0.3f, 1.0f) * (1.f - x) * (1.f - y) +
                    Color4(0.2f, 0.1f, 1.0f, 1.0f) * x * y;

                SpriteComponent.Size = Size2(Size, Size);
                Sprite.set(SpriteComponent);
            }
        }
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
        EngineRenderer*                       Renderer,
        const Ptr<Asset::ShaderLibraryAsset>& ShaderLibrary)
    {
        auto RenderGraph = Renderer->GetRenderGraph();
        auto Builder     = RenderGraph->Reset();

        Builder.AppendPass<RG::ScenePass>(RenderGraph->GetStorage(), ShaderLibrary, m_Scene);

        Builder.Build();
    }
} // namespace Neon::Runtime