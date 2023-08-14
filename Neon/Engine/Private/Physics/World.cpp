#include <EnginePCH.hpp>
#include <Physics/World.hpp>

#include <Runtime/GameEngine.hpp>
#include <Scene/Scene.hpp>
#include <Scene/Component/Physics.hpp>
#include <Runtime/GameEngine.hpp>

namespace Neon::Physics
{
    World::World() :
        m_Dispatcher(&m_CollisionConfiguration),
        m_DynamicsWorld(&m_Dispatcher, &m_Broadphase, &m_Solver, &m_CollisionConfiguration)
    {
        m_DynamicsWorld.setGravity(btVector3(0.0f, -9.8f, 0.0f));
        m_DynamicsWorld.setInternalTickCallback(&WorldPostTickCallback, this, false);

#ifndef NEON_DIST
        m_DynamicsWorld.setDebugDrawer(&m_DebugDrawer);
#endif
    }

    World::~World()
    {
    }

    void World::Update(
        double DeltaTime)
    {
        m_DynamicsWorld.stepSimulation(btScalar(DeltaTime), m_MaxSubSteps, m_FixedTimeStep);
#ifndef NEON_DIST
        m_DynamicsWorld.debugDrawWorld();
#endif
    }

    //

    Vector3 World::GetGravity()
    {
        return FromBullet3(m_DynamicsWorld.getGravity());
    }

    void World::SetGravity(
        const Vector3& Gravity)
    {
        m_DynamicsWorld.setGravity(ToBullet3(Gravity));
    }

    double World::GetFixedTimeStep()
    {
        return m_FixedTimeStep;
    }

    void World::SetFixedTimeStep(
        double FixedTimeStep)
    {
        m_FixedTimeStep = FixedTimeStep;
    }

    int World::GetMaxSubSteps()
    {
        return m_MaxSubSteps;
    }

    void World::SetMaxSubSteps(
        int MaxSubSteps)
    {
        m_MaxSubSteps = MaxSubSteps;
    }

    btIDebugDraw::DebugDrawModes World::GetDebugFlags()
    {
#ifndef NEON_DIST
        return btIDebugDraw::DebugDrawModes(m_DebugDrawer.getDebugMode());
#else
        return {};
#endif
    }

    void World::SetDebugFlags(
        btIDebugDraw::DebugDrawModes Flags)
    {
#ifndef NEON_DIST
        m_DebugDrawer.setDebugMode(Flags);
#endif
    }

    //

    void World::AddPhysicsObject(
        btCollisionObject* PhysicsObject,
        uint32_t           Group,
        uint32_t           Mask)
    {
        if (auto SoftBody = btSoftBody::upcast(PhysicsObject))
        {
            m_DynamicsWorld.addSoftBody(SoftBody, Group, Mask);
        }
        else if (auto RigidBody = btRigidBody::upcast(PhysicsObject))
        {
            m_DynamicsWorld.addRigidBody(RigidBody, Group, Mask);
        }
        else
        {
            m_DynamicsWorld.addCollisionObject(PhysicsObject, Group, Mask);
        }
    }

    void World::RemovePhysicsObject(
        btCollisionObject* PhysicsObject)
    {
        if (auto SoftBody = btSoftBody::upcast(PhysicsObject))
        {
            m_DynamicsWorld.removeSoftBody(SoftBody);
        }
        else if (auto RigidBody = btRigidBody::upcast(PhysicsObject))
        {
            m_DynamicsWorld.removeRigidBody(RigidBody);
        }
        else
        {
            m_DynamicsWorld.removeCollisionObject(PhysicsObject);
        }
    }

    //

    template<typename _CollisionTy, typename _TriggerTy>
    static void SendCollisionCallback(
        btPersistentManifold*    Manifold,
        const btCollisionObject* Body,
        flecs::entity_t          EntityId)
    {
        auto World = Runtime::DefaultGameEngine::Get()->GetScene().GetEntityWorld();
        auto Actor = flecs::entity(*World, EntityId);

        if (!(Body->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE)) [[likely]]
        {
            if (auto Event = Actor.get<_CollisionTy>(); Event && Event->Callback)
            {
                Event->Callback(Manifold);
            }
        }
        else
        {
            if (auto Event = Actor.get<_TriggerTy>(); Event && Event->Callback)
            {
                Event->Callback(Manifold);
            }
        }
    }

    void World::WorldPostTickCallback(
        btDynamicsWorld* BtWorld,
        btScalar         DeltaTimeStep)
    {

        //

        auto PhysicsWorld = static_cast<World*>(BtWorld->getWorldUserInfo());

        auto Dispatcher = BtWorld->getDispatcher();
        auto Manifolds  = Dispatcher->getInternalManifoldPointer();

        for (int i = 0; i < Dispatcher->getNumManifolds(); i++)
        {
            auto Manifold = Manifolds[i];

            auto Body0 = Manifold->getBody0();
            auto Body1 = Manifold->getBody1();

            auto Ent0 = std::bit_cast<flecs::entity_t>(Body0->getUserPointer());
            auto Ent1 = std::bit_cast<flecs::entity_t>(Body1->getUserPointer());
            if (!Ent0 || !Ent1)
            {
                continue;
            }

            auto Tuple = std::make_tuple(Body0, Body1, Manifold);

            // Collision exit
            if (Manifold->getNumContacts() == 0)
            {
                if (PhysicsWorld->m_PreviousTickCollisions.contains(Tuple))
                {
                    SendCollisionCallback<Scene::Component::CollisionExit, Scene::Component::TriggerExit>(Manifold, Body0, Ent0);
                    SendCollisionCallback<Scene::Component::CollisionExit, Scene::Component::TriggerExit>(Manifold, Body1, Ent1);
                    PhysicsWorld->m_PreviousTickCollisions.erase(std::move(Tuple));
                }
            }
            else if (!PhysicsWorld->m_PreviousTickCollisions.contains(Tuple))
            {
                SendCollisionCallback<Scene::Component::CollisionEnter, Scene::Component::TriggerEnter>(Manifold, Body0, Ent0);
                SendCollisionCallback<Scene::Component::CollisionEnter, Scene::Component::TriggerEnter>(Manifold, Body1, Ent1);
                PhysicsWorld->m_PreviousTickCollisions.insert(std::move(Tuple));
            }
            else
            {
                SendCollisionCallback<Scene::Component::CollisionStay, Scene::Component::TriggerStay>(Manifold, Body0, Ent0);
                SendCollisionCallback<Scene::Component::CollisionStay, Scene::Component::TriggerStay>(Manifold, Body1, Ent1);
            }
        }
    }
} // namespace Neon::Physics