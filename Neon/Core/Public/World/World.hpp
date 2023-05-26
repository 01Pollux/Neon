#pragma once

#include <Core/Neon.hpp>
#include <flecs/flecs.h>

namespace Neon
{
    class World
    {
    public:
        template<typename _Ty>
        struct ModuleWrapper
        {
            ModuleWrapper(flecs::world&)
            {
            }
            UPtr<_Ty> Module;
        };

        World();
        NEON_CLASS_NO_COPYMOVE(World);
        ~World();

        /// <summary>
        /// Get world
        /// </summary>
        [[nodiscard]] auto Get() const noexcept
        {
            return m_World.get();
        }

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
        _Ty* Import(
            _Args&&... Args)
        {
            auto Entity     = m_World->import <ModuleWrapper<_Ty>>();
            auto Wrapper    = Entity.get_mut<ModuleWrapper<_Ty>>();
            Wrapper->Module = std::make_unique<_Ty>(*this, std::forward<_Args>(Args)...);
            return Wrapper->Module.get();
        }

        template<typename _Ty>
        flecs::entity ModuleScope()
        {
            return m_World->module<ModuleWrapper<_Ty>>();
        }

        template<typename _Ty>
        [[nodiscard]] _Ty* GetModule()
        {
            auto Wrapper = m_World->get<ModuleWrapper<_Ty>>();
            return Wrapper->Module.get();
        }

    private:
        std::unique_ptr<flecs::world> m_World;
    };
} // namespace Neon