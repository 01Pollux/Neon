#pragma once

#include <Core/Neon.hpp>
#include <Physics/Bullet3.hpp>

#include <Bullet3/btBulletCollisionCommon.h>
#include <Bullet3/btBulletDynamicsCommon.h>
#include <Bullet3/BulletSoftBody/btSoftBody.h>
#include <Bullet3/BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <Bullet3/BulletSoftBody/btSoftRigidDynamicsWorld.h>

#include <unordered_set>

namespace Neon::Physics
{
    class World
    {
        using CollisionData = std::tuple<const btCollisionObject*, const btCollisionObject*, btPersistentManifold*>;
        struct CollisionDataHash
        {
            std::size_t operator()(
                const CollisionData& Data) const
            {
                return std::hash<const btCollisionObject*>()(std::get<0>(Data)) ^
                       std::hash<const btCollisionObject*>()(std::get<1>(Data));
            }
        };
        using CollisionDataSet = std::unordered_set<CollisionData, CollisionDataHash>;

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
            btCollisionObject* RigidBody)
        {
            m_DynamicsWorld.addRigidBody(static_cast<btRigidBody*>(RigidBody));
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

    public:
        /// <summary>
        /// Add a physics object to the physics world.
        /// </summary>
        void AddPhysicsObject(
            btCollisionObject* PhysicsObject,
            uint32_t           Group,
            uint32_t           Mask);

        /// <summary>
        /// Remove a physics object from the physics world.
        /// </summary>
        void RemovePhysicsObject(
            btCollisionObject* PhysicsObject);

    private:
        /// <summary>
        /// Callback for when the physics world ticks.
        /// </summary>
        static void WorldPostTickCallback(
            btDynamicsWorld* World,
            btScalar         DeltaTimeStep);

    private:
        btSoftBodyRigidBodyCollisionConfiguration m_CollisionConfiguration;
        btCollisionDispatcher                     m_Dispatcher;
        btDbvtBroadphase                          m_Broadphase;
        btSequentialImpulseConstraintSolver       m_Solver;
        btSoftRigidDynamicsWorld                  m_DynamicsWorld;

    private:
        CollisionDataSet m_PreviousTickCollisions;

    private:
        double m_FixedTimeStep = 1.0 / 60.0;
        int    m_MaxSubSteps   = 4;
    };
} // namespace Neon::Physics