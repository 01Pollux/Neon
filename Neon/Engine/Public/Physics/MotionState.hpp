#pragma once

#include <Core/Neon.hpp>
#include <Scene/Component/Transform.hpp>
#include <Bullet3/LinearMath/btMotionState.h>
#include <Physics/Bullet3.hpp>

namespace Neon::Physics
{
    class ActorMotionState : public btMotionState
    {
    public:
        ActorMotionState(
            flecs::entity Target) :
            m_Target(std::move(Target))
        {
        }

        void getWorldTransform(
            btTransform& WorldTransform) const override
        {
            if (auto Transform = m_Target.get<Scene::Component::Transform>())
            {
                WorldTransform = ToBullet3(Transform->World);
            }
        }

        void setWorldTransform(
            const btTransform& WorldTransform) override
        {
            if (auto Transform = m_Target.get_mut<Scene::Component::Transform>())
            {
                Transform->World = FromBullet3(WorldTransform);
                m_Target.modified<Scene::Component::Transform>();
            }
        }

    private:
        flecs::entity m_Target;
    };
} // namespace Neon::Physics