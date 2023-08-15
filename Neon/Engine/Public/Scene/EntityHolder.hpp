#pragma once

#include <flecs/flecs.h>
#include <Runtime/GameLogic.hpp>

namespace Neon::Scene
{
    template<typename _Ty>
    class ObjectHolder
    {
    public:
        ObjectHolder() = default;

        ObjectHolder(
            _Ty&& Object) :
            m_Object(std::move(Object)),
            m_EntityId(m_Object.entity())
        {
        }

        ~EntityHolder()
        {
            flecs::entity Entity(Runtime::GameLogic::Get()->GetEntityWorld(), m_EntityId);
            if (Entity.is_alive())
            {
                Entity.destruct();
                m_Object.destruct();
                m_Object   = {};
                m_EntityId = flecs::entity::null();
            }
        }

        _Ty* operator->()
        {
            return &m_Object;
        }

        _Ty& operator*()
        {
            return m_Object;
        }

    private:
        _Ty             m_Object;
        flecs::entity_t m_EntityId;
    };
} // namespace Neon::Scene