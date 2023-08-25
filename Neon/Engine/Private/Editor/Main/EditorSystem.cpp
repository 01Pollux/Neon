#include <EnginePCH.hpp>
#include <Runtime/GameLogic.hpp>
#include <Editor/Main/EditorEngine.hpp>

#include <Editor/Views/Components/EngineComponents.hpp>
#include <Editor/Scene/EditorEntity.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Camera.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Graphs/Standard2D.hpp>

// for testing: TODO remove
#include <Renderer/Material/Material.hpp>
#include <Renderer/Material/Builder.hpp>
#include <Asset/Manager.hpp>
#include <Asset/Types/Shader.hpp>

namespace Neon::Editor
{
    void EditorEngine::AddStandardEditorSystem()
    {
        // Register editor world components
        RegisterEditorWorldComponents();

        // Register component handlers
        AddStandardComponentHandlers();

        // Register editor's main camera
        AddEditorCamera();

        //

        // This is test: TODO remove
        flecs::world World = EditorEngine::Get()->GetLogic()->GetEntityWorld();
        auto         Root  = EditorEngine::Get()->GetEditorRootEntity();

        auto A = World.entity("Unnamed Entity");
        auto B = World.entity("B");
        auto C = World.entity("C");
        auto D = World.entity("D");
        auto E = World.entity("E");
        auto F = World.entity("F");
        auto G = World.entity("G");

        A.child_of(Root);
        {
            B.child_of(A);
            C.child_of(A);
            {
                D.child_of(C);
            }
            E.child_of(A);
        }

        Scene::Component::Transform Tr;
        Tr.World.SetPosition({ 30.0f, 20.0f, 10.0f });
        Tr.World.SetRotationEuler(glm::radians(Vec::Forward<Vector3> * -90.f));
        F.set(Tr);

        A.add<Scene::Editor::HideInEditor>();
        F.child_of(Root);
        G.child_of(Root);

        {

            using ShaderAssetTaskPtr = Asset::AssetTaskPtr<Asset::ShaderAsset>;

            auto               RocketShaderGuid = Asset::Handle::FromString("7427990f-9be1-4a23-aad5-1b99f00c29fd");
            ShaderAssetTaskPtr RocketShader     = Asset::Manager::Load(RocketShaderGuid);

            Renderer::GBufferMaterialBuilder BaseSpriteMaterial;

            BaseSpriteMaterial
                .Rasterizer(Renderer::MaterialStates::Rasterizer::CullNone)
                .Topology(RHI::PrimitiveTopologyCategory::Triangle)
                .RootSignature(
                    RHI::RootSignatureBuilder()
                        .AddConstantBufferView("g_FrameData", 0, 1, RHI::ShaderVisibility::All)
                        .AddShaderResourceView("g_SpriteData", 0, 1, RHI::ShaderVisibility::All)
                        .AddDescriptorTable(
                            RHI::RootDescriptorTable()
                                .AddSrvRange("p_SpriteTextures", 0, 2, 1, true),
                            RHI::ShaderVisibility::Pixel)
                        .SetFlags(RHI::ERootSignatureBuilderFlags::AllowInputLayout)
                        .AddStandardSamplers()
                        .Build());

            RHI::MShaderCompileFlags Flags;
#if NEON_DEBUG
            Flags.Set(RHI::EShaderCompileFlags::Debug);
#endif

            BaseSpriteMaterial
                .VertexShader(RocketShader->LoadShader({ .Stage = RHI::ShaderStage::Vertex, .Flags = Flags }))
                .PixelShader(RocketShader->LoadShader({ .Stage = RHI::ShaderStage::Pixel, .Flags = Flags }));

            auto WorldMaterial = Renderer::IMaterial::Create(std::move(BaseSpriteMaterial));

            auto WorldTexture = RHI::ITexture::GetDefault(RHI::DefaultTextures::White_2D);
            WorldMaterial->SetTexture("p_SpriteTextures", WorldTexture);

            auto WallCreate =
                [&](const char* Name, const Vector3& Position)
            {
                auto Wall = World.entity(Name).child_of(B);
                {
                    Scene::Component::Sprite SpriteComponent;
                    {
                        SpriteComponent.MaterialInstance = WorldMaterial;
                        SpriteComponent.SpriteSize       = Vector2(100.f, 2.35f);
                    }
                    Wall.set(std::move(SpriteComponent));
                    Wall.add<Scene::Component::Sprite::MainRenderer>();

                    Scene::Component::Transform Transform;
                    {
                        Transform.World.SetPosition(Position);
                    }
                    Wall.set(std::move(Transform));

                    Wall.add<Scene::Component::SceneEntity>();

                    if (0)
                    {
                        Wall.set(Scene::Component::CollisionShape{
                            std::make_unique<btBoxShape>(btVector3(100.f, 2.35, 200.f) / 2.f) });

                        auto StaticBody = Scene::Component::CollisionObject::AddStaticBody(Wall);
                        StaticBody->setCustomDebugColor(Physics::ToBullet3(Colors::Green));
                    }
                }
            };

            WallCreate("Ceiling", Vec::Forward<Vector3> * 0.f);
        }
    }

    //

    void EditorEngine::RegisterEditorWorldComponents()
    {
        flecs::world World = GetLogic()->GetEntityWorld();
        m_EditorRootEntity = World.entity(EditorRootEntityName);

        NEON_REGISTER_FLECS(Scene::Editor::HideInEditor);
        NEON_REGISTER_FLECS(Scene::Editor::SelectedForEditor);
        NEON_REGISTER_FLECS(Scene::Editor::WorldEditorMode);
        NEON_REGISTER_FLECS(Scene::Editor::EditorSceneDoNotRemove);

        // By default, editor world is in editor mode
        World.add<Scene::Editor::WorldEditorMode>();
    }

    //

    void EditorEngine::AddStandardComponentHandlers()
    {
        RegisterStandardComponentHandler<CameraComponentHandler, Scene::Component::Camera>();
        RegisterStandardComponentHandler<PhysicsComponentHandler, Scene::Component::CollisionObject>();
        RegisterStandardComponentHandler<SpriteComponentHandler, Scene::Component::Sprite>();
        RegisterStandardComponentHandler<TransformComponentHandler, Scene::Component::Transform>();
    }

    //

    void EditorEngine::AddEditorCamera()
    {
        flecs::world World = GetLogic()->GetEntityWorld();

        auto RootEntity = EditorEngine::Get()->GetEditorRootEntity();
        auto Camera     = World.entity("Editor Camera").child_of(RootEntity);

        Scene::Component::Camera CameraComponent(Scene::Component::CameraType::Orthographic);
        {
            RG::CreateStandard2DRenderGraph(CameraComponent, Camera);

            CameraComponent.Viewport.OrthographicSize = 50.0f;
            CameraComponent.Viewport.NearPlane        = -1.0f;
            CameraComponent.Viewport.FarPlane         = 20.0f;
        }
        Camera.set(std::move(CameraComponent));

        Scene::Component::Transform TransformComponent;
        {
            TransformComponent.World.SetRotationEuler(glm::radians(Vec::Right<Vector3> * -90.f));
            TransformComponent.World.SetPosition(Vec::Backward<Vector3> * 10.f);
        }
        Camera.set(std::move(TransformComponent));

        Camera.add<Scene::Editor::EditorSceneDoNotRemove>();

        RootEntity.add<Scene::Component::MainCamera>(Camera);
    }

    //

    flecs::entity EditorEngine::GetEditorRootEntity() const
    {
        flecs::world World = GetLogic()->GetEntityWorld();
        return flecs::entity(World, m_EditorRootEntity);
    }

    flecs::entity EditorEngine::GetEditorActiveRootEntity() const
    {
        return GetRootEntity(IsInEditorMode());
    }

    flecs::entity EditorEngine::GetRootEntity(
        bool EditorMode) const
    {
        return EditorMode ? GetEditorRootEntity() : GetLogic()->GetEntityWorld().GetRootEntity();
    }

    bool EditorEngine::IsInEditorMode() const
    {
        auto World = GetLogic()->GetEntityWorld();
        return World.GetWorld().has<Scene::Editor::WorldEditorMode>();
    }
} // namespace Neon::Editor