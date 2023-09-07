#pragma once

#include <Scene/Component/Component.hpp>

namespace Neon::Scene::Editor
{
    /// <summary>
    /// Entites that have this component are content browser items
    /// </summary>
    struct ContentBrowserItem
    {
        /// <summary>
        /// Type of the content browser item
        /// </summary>
        enum class ItemType
        {
            Directory,
            File
        };

        /// <summary>
        /// Type of the file operation
        /// </summary>
        enum class ItemOpType
        {
            None,
            Selected,
            SelectedCut,
            SelectedCopy
        };

        NEON_EXPORT_FLECS_ENUM(ItemType, "Editor::ContentBrowserItem::ItemType")
        {
        }

        NEON_EXPORT_FLECS_ENUM(ItemOpType, "Editor::ContentBrowserItem::ItemOpType")
        {
        }

        NEON_EXPORT_FLECS(ContentBrowserItem, "Editor::ContentBrowserItem")
        {
        }
    };
} // namespace Neon::Scene::Editor