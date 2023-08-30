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
#include <Scene/Component/Script.hpp>

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
    }

    //

    void EditorEngine::RegisterEditorWorldComponents()
    {
        NEON_REGISTER_FLECS(Scene::Editor::SelectedForEditor);
        NEON_REGISTER_FLECS(Scene::Editor::WorldEditorMode);
        NEON_REGISTER_FLECS(Scene::Editor::EditorSceneDoNotRemove);
        NEON_REGISTER_FLECS(Scene::Editor::EditorMainCamera);

        flecs::world World = Scene::EntityWorld::Get();

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
        flecs::entity Root  = Scene::EntityWorld::GetRootEntity();
        flecs::world  World = Scene::EntityWorld::Get();

        auto Camera = World.entity("Editor Camera").child_of(Root);

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

        World.add<Scene::Editor::EditorMainCamera>(Camera);
    }

    //

    const Scene::RuntimeScene& EditorEngine::GetRuntimeScene() const
    {
        return m_RuntimeScene;
    }

    const Scene::RuntimeScene& EditorEngine::GetEditorScene() const
    {
        return m_EditorScene;
    }

    Scene::EntityHandle EditorEngine::GetMainCamera(
        bool EditorCamera) const
    {
        flecs::world World = Scene::EntityWorld::Get();
        return EditorCamera ? World.target<Scene::Editor::EditorMainCamera>()
                            : World.target<Scene::Component::MainCamera>();
    }

    bool EditorEngine::IsInEditorMode() const
    {
        flecs::world World = Scene::EntityWorld::Get();
        return World.has<Scene::Editor::WorldEditorMode>();
    }
} // namespace Neon::Editor