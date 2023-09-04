#pragma once

#include <Renderer/Material/Material.hpp>

namespace Neon::Renderer
{
    struct SharedMaterials
    {
        enum class Type : uint8_t
        {
            LitSprite
        };

        /// <summary>
        /// Initialize the default materials.
        /// </summary>
        static void Initialize();

        /// <summary>
        /// Shutdown the default materials.
        /// </summary>
        static void Shutdown();

        /// <summary>
        /// Get default material for current type.
        /// </summary>
        [[nodiscard]] static Ptr<IMaterial> Get(
            Type Ty);
    };
} // namespace Neon::Renderer