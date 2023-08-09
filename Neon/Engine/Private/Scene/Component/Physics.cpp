#include <EnginePCH.hpp>
#include <Physics/MotionState.hpp>
#include <Scene/Component/Physics.hpp>
#include <Bullet3/btBulletDynamicsCommon.h>

namespace Neon::Scene::Component
{
    btCollisionObject* CollisionObject::AddStaticBody(
        flecs::entity Target,
        uint32_t      Group,
        uint32_t      Mask)
    {
        auto Shape       = Target.get<CollisionShape>();
        auto MotionState = new Physics::ActorMotionState(Target);

        auto Object = std::make_unique<btRigidBody>(0., MotionState, Shape->BulletShape.get());
        Object->setUserPointer(std::bit_cast<void*>(Target.id()));
        Object->setCollisionFlags(Object->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);

        auto ObjectPtr = Object.get();

        Target.set(CollisionObject{
            .BulletObject = std::move(Object),
            .Group        = Group,
            .Mask         = Mask });

        return ObjectPtr;
    }

    btCollisionObject* CollisionObject::AddRigidBody(
        flecs::entity Target,
        float         Mass,
        uint32_t      Group,
        uint32_t      Mask)
    {
        auto Shape       = Target.get<CollisionShape>();
        auto MotionState = new Physics::ActorMotionState(Target);

        btVector3 LocalInertia;
        if (Mass > 0.f)
        {
            Shape->BulletShape->calculateLocalInertia(Mass, LocalInertia);
        }

        auto Object = std::make_unique<btRigidBody>(Mass, MotionState, Shape->BulletShape.get(), LocalInertia);
        Object->setUserPointer(std::bit_cast<void*>(Target.id()));

        auto ObjectPtr = Object.get();

        Target.set(CollisionObject{
            .BulletObject = std::move(Object),
            .Group        = Group,
            .Mask         = Mask });

        return ObjectPtr;
    }

    btCollisionObject* CollisionObject::AddKinematicBody(
        flecs::entity Target,
        float         Mass,
        uint32_t      Group,
        uint32_t      Mask)
    {
        auto Shape       = Target.get<CollisionShape>();
        auto MotionState = new Physics::ActorMotionState(Target);

        btVector3 LocalInertia;
        if (Mass > 0.f)
        {
            Shape->BulletShape->calculateLocalInertia(Mass, LocalInertia);
        }

        auto Object = std::make_unique<btRigidBody>(Mass, MotionState, Shape->BulletShape.get(), LocalInertia);
        Object->setUserPointer(std::bit_cast<void*>(Target.id()));
        Object->setCollisionFlags(Object->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);

        auto ObjectPtr = Object.get();

        Target.set(CollisionObject{
            .BulletObject = std::move(Object),
            .Group        = Group,
            .Mask         = Mask });

        return ObjectPtr;
    }
} // namespace Neon::Scene::Component