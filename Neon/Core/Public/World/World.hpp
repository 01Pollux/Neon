#pragma once

#include <Core/Neon.hpp>
#include <flecs/flecs.h>

namespace Neon
{
    class World
    {
    public:
        World();
        NEON_CLASS_NO_COPYMOVE(World);
        ~World();

        /// <summary>
        /// Get world
        /// </summary>
        [[nodiscard]] auto operator->() const noexcept
        {
            return m_World.get();
        }

        /// <summary>
        /// Get world
        /// </summary>
        [[nodiscard]] auto operator->() noexcept
        {
            return m_World.get();
        }

    private:
        std::unique_ptr<flecs::world> m_World;
    };
} // namespace Neon