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

        flecs::world World = Scene::EntityWorld::Get();

        // By default, editor world is in editor mode
        World.add<Scene::Editor::WorldEditorMode>();
    }

    //

    flecs::entity EditorEngine::GetEditorCamera() const
    {
        return m_EditorCamera;
    }

    flecs::entity EditorEngine::GetActiveCamera() const
    {
        return m_EditorCamera;
    }

    //

    flecs::entity EditorEngine::GetActiveSceneTag() const
    {
        return IsInEditorMode() ? Scene::EntityWorld::GetCurrentScenerRoot() : m_RuntimeScene.GetRoot().Get();
    }

    bool EditorEngine::IsInEditorMode() const
    {
        flecs::world World = Scene::EntityWorld::Get();
        return World.has<Scene::Editor::WorldEditorMode>();
    }

    flecs::entity EditorEngine::CreateEditorEntity(
        const char* Name)
    {
        return Scene::EntityHandle::Create(m_EditorScene.GetRoot(), Name);
    }

    flecs::entity EditorEngine::CreateEditorEntity(
        const flecs::entity& Parent,
        const char*          Name)
    {
        return Scene::EntityHandle::Create(m_EditorScene.GetRoot(), Parent, Name);
    }
} // namespace Neon::Editor