#pragma once

#include <Scene/Component/Component.hpp>

namespace Neon::Scene::Editor
{
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
} // namespace Neon::Scene::Editor