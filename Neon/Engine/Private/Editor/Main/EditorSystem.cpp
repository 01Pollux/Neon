#include <EnginePCH.hpp>
#include <Runtime/GameLogic.hpp>
#include <Editor/Main/EditorEngine.hpp>
#include <Editor/Scene/EditorEntity.hpp>

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
        NEON_REGISTER_FLECS(Scene::Editor::SceneCameraCanMove);

        flecs::world World = Scene::EntityWorld::Get();

        // By default, editor world is in editor mode
        World.add<Scene::Editor::WorldEditorMode>();
    }

    //

    flecs::entity EditorEngine::GetEditorCamera() const
    {
        return m_EditorCamera;
    }

    //

    flecs::entity EditorEngine::GetActiveSceneTag() const
    {
        return IsInEditorMode() ? Scene::EntityWorld::GetCurrentScenerRoot() : m_RuntimeScene.GetRoot().Get();
    }

    flecs::entity EditorEngine::GetEditorScene() const
    {
        return m_EditorScene.GetRoot();
    }

    bool EditorEngine::IsInEditorMode() const
    {
        flecs::world World = Scene::EntityWorld::Get();
        return World.has<Scene::Editor::WorldEditorMode>();
    }

    flecs::entity EditorEngine::CreateEditorEntity(
        const char* Name)
    {
        return Scene::EntityHandle::Create(GetEditorScene(), Name);
    }

    flecs::entity EditorEngine::CreateEditorEntity(
        const flecs::entity& Parent,
        const char*          Name)
    {
        return Scene::EntityHandle::Create(GetEditorScene(), Parent, Name);
    }
} // namespace Neon::Editor