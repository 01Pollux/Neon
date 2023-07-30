#include <EnginePCH.hpp>
#include <Physics/World.hpp>

namespace Neon::Physics
{
    World::World() :
        m_Dispatcher(&m_CollisionConfiguration),
        m_DynamicsWorld(&m_Dispatcher, &m_Broadphase, &m_Solver, &m_CollisionConfiguration)
    {
        m_DynamicsWorld.setGravity(btVector3(0.0f, -1.1f, 0.0f));
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
} // namespace Neon::Physics