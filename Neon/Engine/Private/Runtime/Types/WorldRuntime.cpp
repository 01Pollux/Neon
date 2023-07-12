#include <EnginePCH.hpp>
#include <Runtime/Types/WorldRuntime.hpp>

#include <Resource/Manager.hpp>
#include <Resource/Pack.hpp>

#include <Runtime/GameEngine.hpp>
#include <Runtime/Pipeline.hpp>

#include <Window/Window.hpp>
#include <RHI/Swapchain.hpp>

//

#include <Scene/Component/Camera.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Sprite.hpp>
#include <Renderer/Material/Builder.hpp>

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
            [this]
            {
                RHI::ISwapchain::Get()->PrepareFrame();
            });

        Pipeline->Attach(
            "Render",
            [this]
            {
                m_Scene.Render();
            });

        Pipeline->Attach(
            "PostRender",
            [this]
            {
                RHI::ISwapchain::Get()->Present();
            });

        Engine->SetPipeline(std::move(Pipeline));

        //

        using namespace Renderer;

        RenderMaterialBuilder MatBuilder;

        MatBuilder.ShaderLibrary(ShaderLib)
            .VertexShader(Asset::ShaderModuleId(0))
            .PixelShader(Asset::ShaderModuleId(0))
            .Rasterizer(MaterialStates::Rasterizer::CullNone)
            .DepthStencil(MaterialStates::DepthStencil::None)
            .RenderTarget(0, "Base Color", RHI::EResourceFormat::R8G8B8A8_UNorm)
            .Topology(RHI::PrimitiveTopologyCategory::Triangle);

        {
            auto& VarMap = MatBuilder.VarMap();

            VarMap.Add("g_FrameData", { 0, 0 }, MaterialVarType::Buffer)
                .Visibility(RHI::ShaderVisibility::All);

            VarMap.Add("g_SpriteData", { 0, 1 }, MaterialVarType::Resource)
                .Visibility(RHI::ShaderVisibility::All);

            VarMap.Add("p_SpriteTextures", { 0, 0 }, MaterialVarType::Resource)
                .Visibility(RHI::ShaderVisibility::Pixel)
                .Flags(EMaterialVarFlags::Instanced, true);

            for (uint32_t i : std::ranges::iota_view(0u, uint32_t(MaterialStates::Sampler::_Last)))
            {
                auto Name = StringUtils::Format("p_StaticSampler_{}", i);
                VarMap.AddStaticSampler(Name, { i, 0 }, RHI::ShaderVisibility::Pixel, MaterialStates::Sampler(i));
            }
        }

        auto Material = IMaterial::Create(MatBuilder);

        //

        Ptr<IMaterial> RandomInstances[]{
            Material->CreateInstance(),
            Material->CreateInstance(),
            Material->CreateInstance(),
            Material->CreateInstance(),
            Material
        };

        Ptr<RHI::ITexture> RandomTextures[]{
            RHI::ITexture::GetDefault(RHI::DefaultTextures::Magenta_2D),
            RHI::ITexture::GetDefault(RHI::DefaultTextures::White_2D),
        };

        //

        for (float y = -1.f; y < 1.f; y += .2f)
        {
            for (float x = -1.f; x < 1.f; x += .2f)
            {
                constexpr float Size = 0.2f;

                auto Sprite = m_Scene->entity();

                Scene::Component::Transform TransformComponent;
                TransformComponent.Local.SetPosition(Vector3(x, y, 5.f));
                TransformComponent.World.SetPosition(Vector3(x, y, 5.f));
                Sprite.set(TransformComponent);

                Scene::Component::Sprite SpriteComponent;

                // SpriteComponent.ModulationColor =
                //     Color4(0.0f, 1.0f, 0.3f, 1.0f) * (1.f - x) * (1.f - y) +
                //     Color4(0.2f, 0.1f, 1.0f, 1.0f) * x * y;

                SpriteComponent.Size             = Size2(Size, Size);
                SpriteComponent.MaterialInstance = RandomInstances[std::rand() % std::size(RandomInstances)];

                SpriteComponent.MaterialInstance->SetTexture("p_SpriteTextures", RandomTextures[std::rand() % std::size(RandomTextures)]);

                Sprite.set(SpriteComponent);
            }
        }

        //

        auto CameraActor        = m_Scene.CreateEntity(Scene::EntityType::Camera3D, "Main Camera");
        auto CameraComponent    = CameraActor.get_mut<Scene::Component::Camera>();
        CameraComponent->LookAt = Vector3(0.f, 0.f, 5.f);

        m_Scene->set<Scene::Component::MainCamera>({ CameraActor });
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