#include <EnginePCH.hpp>
#include <Scene/Physics/GamePhysics.hpp>

namespace Neon::Scene
{
    PhysicsWorld::PhysicsWorld() :
        m_Dispatcher(&m_CollisionConfiguration),
        m_DynamicsWorld(&m_Dispatcher, &m_Broadphase, &m_Solver, &m_CollisionConfiguration)
    {
    }

    PhysicsWorld::~PhysicsWorld()
    {
    }

    void PhysicsWorld::Update(
        double DeltaTime)
    {
        m_DynamicsWorld.stepSimulation(btScalar(DeltaTime), m_MaxSubSteps, m_FixedTimeStep);
    }

    //

    Vector3 PhysicsWorld::GetGravity()
    {
        auto Vec = m_DynamicsWorld.getGravity();
        return Vector3(Vec.x(), Vec.y(), Vec.z());
    }

    void PhysicsWorld::SetGravity(
        const Vector3& Gravity)
    {
        m_DynamicsWorld.setGravity(btVector3(Gravity.x, Gravity.y, Gravity.z));
    }

    double PhysicsWorld::GetFixedTimeStep()
    {
        return m_FixedTimeStep;
    }

    void PhysicsWorld::SetFixedTimeStep(
        double FixedTimeStep)
    {
        m_FixedTimeStep = FixedTimeStep;
    }

    int PhysicsWorld::SetMaxSubSteps()
    {
        return m_MaxSubSteps;
    }

    void PhysicsWorld::SetMaxSubSteps(
        int MaxSubSteps)
    {
        m_MaxSubSteps = MaxSubSteps;
    }
} // namespace Neon::Scene