#pragma once

#include <Core/Neon.hpp>
#include <Physics/Bullet3.hpp>

#include <Bullet3/btBulletCollisionCommon.h>
#include <Bullet3/btBulletDynamicsCommon.h>
#include <Bullet3/BulletSoftBody/btSoftBody.h>
#include <Bullet3/BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <Bullet3/BulletSoftBody/btSoftRigidDynamicsWorld.h>

namespace Neon::Physics
{
    class World
    {
    public:
        World();

        NEON_CLASS_NO_COPY(World);
        NEON_CLASS_NO_MOVE(World);

        ~World();

        /// <summary>
        /// Update the physics world.
        /// </summary>
        void Update(
            double DeltaTime);

    public:
        /// <summary>
        /// Add a rigid body to the physics world.
        /// </summary>
        /// <param name="RigidBody"></param>
        void TmpAddRigidBody(
            btRigidBody* RigidBody)
        {
            m_DynamicsWorld.addRigidBody(RigidBody);
        }

    public:
        /// <summary>
        /// Get the gravity of the physics world.
        /// </summary>
        [[nodiscard]] Vector3 GetGravity();

        /// <summary>
        /// Set the gravity of the physics world.
        /// </summary>
        void SetGravity(
            const Vector3& Gravity);

        /// <summary>
        /// Get the fixed time step of the physics world.
        /// </summary>
        [[nodiscard]] double GetFixedTimeStep();

        /// <summary>
        /// Set the fixed time step of the physics world.
        /// </summary>
        void SetFixedTimeStep(
            double FixedTimeStep);

        /// <summary>
        /// Get the maximum number of sub steps of the physics world.
        /// </summary>
        [[nodiscard]] int SetMaxSubSteps();

        /// <summary>
        /// Set the maximum number of sub steps of the physics world.
        /// </summary>
        void SetMaxSubSteps(
            int MaxSubSteps);

    private:
        btSoftBodyRigidBodyCollisionConfiguration m_CollisionConfiguration;
        btCollisionDispatcher                     m_Dispatcher;
        btDbvtBroadphase                          m_Broadphase;
        btSequentialImpulseConstraintSolver       m_Solver;
        btSoftRigidDynamicsWorld                  m_DynamicsWorld;

    private:
        double m_FixedTimeStep = 1.0 / 60.0;
        int    m_MaxSubSteps   = 4;
    };
} // namespace Neon::Physics