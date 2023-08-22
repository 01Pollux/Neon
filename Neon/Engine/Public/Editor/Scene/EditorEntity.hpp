#pragma once

#include <Scene/Component/Component.hpp>

namespace Neon::Scene::Editor
{
    /// <summary>
    /// Entities that exists in _EditorRoot/_Root and have this tag will be hidden in editor.
    /// </summary>
    struct HideInEditor
    {
    };

    /// <summary>
    /// Entites that have this tag will be selected in editor.
    /// </summary>
    struct SelectedForEditor
    {
    };
} // namespace Neon::Scene::Editor