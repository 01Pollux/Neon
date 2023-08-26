#pragma once

#include <Scene/Component/Component.hpp>

namespace Neon::Scene::Editor
{
    // TODO: remove this
    /// <summary>
    /// Entities that exists in Root and have this tag will be hidden in editor. (not visible in editor's scene)
    /// </summary>
    struct HideInEditor
    {
        NEON_EXPORT_FLECS(HideInEditor, "Editor::HideInEditor")
        {
        }
    };

    /// <summary>
    /// Entites that have this tag will be selected in editor.
    /// </summary>
    struct SelectedForEditor
    {
        NEON_EXPORT_FLECS(SelectedForEditor, "Editor::SelectedForEditor")
        {
            Component.add(flecs::Exclusive);
        }
    };

    /// <summary>
    /// World tag that indicates that the world is in editor mode.
    /// </summary>
    struct WorldEditorMode
    {
        NEON_EXPORT_FLECS(WorldEditorMode, "Editor::WorldEditorMode")
        {
        }
    };

    /// <summary>
    /// Tag for entities that should not be removed from editor's scene.
    /// </summary>
    struct EditorSceneDoNotRemove
    {
        NEON_EXPORT_FLECS(EditorSceneDoNotRemove, "Editor::EditorSceneDoNotRemove")
        {
        }
    };

    /// <summary>
    /// The main camera of the editor.
    /// </summary>
    struct EditorMainCamera
    {
        NEON_EXPORT_FLECS(EditorMainCamera, "Editor::EditorMainCamera")
        {
        }
    };
} // namespace Neon::Scene::Editor