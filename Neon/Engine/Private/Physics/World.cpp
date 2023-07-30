#include <EnginePCH.hpp>
#include <Physics/World.hpp>

namespace Neon::Physics
{
    World::World() :
        m_Dispatcher(&m_CollisionConfiguration),
        m_DynamicsWorld(&m_Dispatcher, &m_Broadphase, &m_Solver, &m_CollisionConfiguration)
    {
        m_DynamicsWorld.setGravity(btVector3(0.0f, -9.8f, 0.0f));
    }

    World::~World()
    {
    }

    void World::Update(
        double DeltaTime)
    {
        m_DynamicsWorld.stepSimulation(btScalar(DeltaTime), m_MaxSubSteps, m_FixedTimeStep);
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

    int World::SetMaxSubSteps()
    {
        return m_MaxSubSteps;
    }

    void World::SetMaxSubSteps(
        int MaxSubSteps)
    {
        m_MaxSubSteps = MaxSubSteps;
    }

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
} // namespace Neon::Physics