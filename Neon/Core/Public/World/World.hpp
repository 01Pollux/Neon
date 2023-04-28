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

        template<typename _Ty, typename... _Args>
        void Import(
            _Args&&... Args)
        {
            _Ty* Module = m_World->import <_Ty>().get_mut<_Ty>();
            if constexpr (sizeof...(Args) > 1)
            {
                std::destroy_at(Module);
                std::construct_at(Module, *m_World, std::forward<_Args>(Args)...);
            }
        }

    private:
        std::unique_ptr<flecs::world> m_World;
    };
} // namespace Neon