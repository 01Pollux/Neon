#pragma once

#include <Scene/Component/Component.hpp>

namespace Neon::Scene::Editor
{
    /// <summary>
    /// Entities that exists in _EditorRoot/_Root and have this tag will be hidden in editor.
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
} // namespace Neon::Scene::Editor