#pragma once

#include <Scene/Component/Component.hpp>

namespace Neon::Scene::Editor
{
    /// <summary>
    /// Target for editor's scene, references current entity being edited.
    /// </summary>
    struct SelectedForEditor
    {
        NEON_EXPORT_FLECS(SelectedForEditor, "Editor::SelectedForEditor")
        {
            Component.add(flecs::Exclusive);
        }
    };

    /// <summary>
    /// Tag for entities that are assets and not part of the scene.
    /// </summary>
    struct EditorAsset
    {
        NEON_EXPORT_FLECS(EditorAsset, "Editor::EditorAsset")
        {
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
    /// Tag for only editor camera, indicating that we can move the camera (mouse is inside window).
    /// </summary>
    struct SceneCameraCanMove
    {
        NEON_EXPORT_FLECS(SceneCameraCanMove, "Editor::SceneCameraCanMove")
        {
        }
    };
} // namespace Neon::Scene::Editor