#include <EnginePCH.hpp>
#include <Physics/MotionState.hpp>
#include <Scene/Component/Physics.hpp>
#include <Bullet3/btBulletDynamicsCommon.h>

namespace Neon::Scene::Component
{
    Actor CollisionObject::AddStaticBody(
        Actor    Target,
        uint32_t Group,
        uint32_t Mask)
    {
        auto Shape       = Target.get<CollisionShape>();
        auto MotionState = new Physics::ActorMotionState(Target);

        auto Object = std::make_unique<btRigidBody>(0., MotionState, Shape->BulletShape.get());
        Object->setCollisionFlags(Object->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);

        return Target.set(CollisionObject{
            .BulletObject = std::move(Object),
            .Group        = Group,
            .Mask         = Mask });
    }

    Actor CollisionObject::AddRigidBody(
        Actor    Target,
        float    Mass,
        uint32_t Group,
        uint32_t Mask)
    {
        auto Shape       = Target.get<CollisionShape>();
        auto MotionState = new Physics::ActorMotionState(Target);

        btVector3 LocalInertia;
        if (Mass > 0.f)
        {
            Shape->BulletShape->calculateLocalInertia(Mass, LocalInertia);
        }

        auto Object = std::make_unique<btRigidBody>(Mass, MotionState, Shape->BulletShape.get(), LocalInertia);

        return Target.set(CollisionObject{
            .BulletObject = std::move(Object),
            .Group        = Group,
            .Mask         = Mask });
    }

    Actor CollisionObject::AddKinematicBody(
        Actor    Target,
        float    Mass,
        uint32_t Group,
        uint32_t Mask)
    {
        auto Shape       = Target.get<CollisionShape>();
        auto MotionState = new Physics::ActorMotionState(Target);

        btVector3 LocalInertia;
        if (Mass > 0.f)
        {
            Shape->BulletShape->calculateLocalInertia(Mass, LocalInertia);
        }

        auto Object = std::make_unique<btRigidBody>(Mass, MotionState, Shape->BulletShape.get(), LocalInertia);
        Object->setCollisionFlags(Object->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);

        return Target.set(CollisionObject{
            .BulletObject = std::move(Object),
            .Group        = Group,
            .Mask         = Mask });
    }
} // namespace Neon::Scene::Component