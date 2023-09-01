#include <EnginePCH.hpp>
#include <Runtime/GameLogic.hpp>
#include <Editor/Main/EditorEngine.hpp>

#include <Editor/Scene/EditorEntity.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Camera.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Graphs/Standard2D.hpp>

namespace Neon::Editor
{
    void EditorEngine::AddStandardEditorSystem()
    {
        // Register editor world components
        RegisterEditorWorldComponents();

        // Register editor's main camera
        AddEditorCamera();
    }

    //

    void EditorEngine::RegisterEditorWorldComponents()
    {
        NEON_REGISTER_FLECS(Scene::Editor::SelectedForEditor);
        NEON_REGISTER_FLECS(Scene::Editor::EditorAsset);
        NEON_REGISTER_FLECS(Scene::Editor::WorldEditorMode);
        NEON_REGISTER_FLECS(Scene::Editor::EditorSceneDoNotRemove);
        NEON_REGISTER_FLECS(Scene::Editor::EditorMainCamera);

        flecs::world World = Scene::EntityWorld::Get();

        // By default, editor world is in editor mode
        World.add<Scene::Editor::WorldEditorMode>();
    }

    //

    void EditorEngine::AddEditorCamera()
    {
        flecs::world  World  = Scene::EntityWorld::Get();
        flecs::entity Camera = World.entity("_Editor Camera_");
        m_EditorCamera       = Camera;

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

        m_EditorScene.ApplyToRoot(Neon::Scene::RuntimeScene::MergeType::Replace);
    }

    //

    flecs::entity EditorEngine::GetActiveSceneTag() const
    {
        return IsInEditorMode() ? Scene::EntityWorld::GetCurrentSceneTag() : m_EditorScene.GetTag().Get();
    }

    const Scene::RuntimeScene& EditorEngine::GetCurrentScene() const
    {
        return m_EditorScene;
    }

    flecs::entity EditorEngine::GetEditorCamera() const
    {
        return m_EditorCamera;
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