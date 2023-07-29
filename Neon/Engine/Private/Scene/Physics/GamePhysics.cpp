#include <EnginePCH.hpp>
#include <Scene/Physics/GamePhysics.hpp>

namespace Neon::Scene
{
    PhysicsWorld::PhysicsWorld() :
        m_CollisionConfiguration(),
        m_Dispatcher(&m_CollisionConfiguration),
        m_Broadphase(),
        m_Solver(),
        m_DynamicsWorld(&m_Dispatcher, &m_Broadphase, &m_Solver, &m_CollisionConfiguration)
    {
    }

    PhysicsWorld::~PhysicsWorld()
    {
    }
} // namespace Neon::Scene