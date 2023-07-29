#pragma once

#include <Core/Neon.hpp>

#include <Bullet3/btBulletCollisionCommon.h>
#include <Bullet3/btBulletDynamicsCommon.h>
#include <Bullet3/BulletSoftBody/btSoftBody.h>
#include <Bullet3/BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <Bullet3/BulletSoftBody/btSoftRigidDynamicsWorld.h>

namespace Neon::Scene
{
    class PhysicsWorld
    {
    public:
        PhysicsWorld();

        NEON_CLASS_NO_COPY(PhysicsWorld);
        NEON_CLASS_NO_MOVE(PhysicsWorld);

        ~PhysicsWorld();

    private:
        btSoftBodyRigidBodyCollisionConfiguration m_CollisionConfiguration;
        btCollisionDispatcher                     m_Dispatcher;
        btDbvtBroadphase                          m_Broadphase;
        btSequentialImpulseConstraintSolver       m_Solver;
        btSoftRigidDynamicsWorld                  m_DynamicsWorld;
    };
} // namespace Neon::Scene