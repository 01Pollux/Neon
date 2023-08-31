#pragma once

#include <Physics/Bullet3.hpp>
#include <Scene/Component/Component.hpp>

#include <Bullet3/btBulletDynamicsCommon.h>
#include <Bullet3/btBulletCollisionCommon.h>

#include <Bullet3/BulletCollision/CollisionDispatch/btGhostObject.h>
#include <Bullet3/BulletDynamics/Dynamics/btRigidBody.h>
#include <Bullet3/BulletSoftBody/btSoftBody.h>

#include <Bullet3/BulletCollision/CollisionShapes/btBox2dShape.h>
#include <Bullet3/BulletCollision/CollisionShapes/btBoxShape.h>
#include <Bullet3/BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <Bullet3/BulletCollision/CollisionShapes/btConeShape.h>
#include <Bullet3/BulletCollision/CollisionShapes/btConvexHullShape.h>
#include <Bullet3/BulletCollision/CollisionShapes/btConvexShape.h>
#include <Bullet3/BulletCollision/CollisionShapes/btCylinderShape.h>
#include <Bullet3/BulletCollision/CollisionShapes/btEmptyShape.h>
#include <Bullet3/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <Bullet3/BulletCollision/CollisionShapes/btMultiSphereShape.h>
#include <Bullet3/BulletCollision/CollisionShapes/btShapeHull.h>
#include <Bullet3/BulletCollision/CollisionShapes/btSphereShape.h>
#include <Bullet3/BulletCollision/CollisionShapes/btStaticPlaneShape.h>
#include <Bullet3/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h>
#include <Bullet3/BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <Bullet3/BulletCollision/CollisionShapes/btUniformScalingShape.h>

#include <functional>

namespace Neon::Scene::Component
{
    using CollisionCallback = std::move_only_function<void(btPersistentManifold* Manifold)>;

    /// <summary>
    /// Called when a collision occurs.
    /// </summary>
    struct CollisionEnter
    {
        NEON_EXPORT_FLECS(CollisionEnter, "Collision Enter")
        {
        }

        mutable CollisionCallback Callback;
    };

    /// <summary>
    /// Called when a collision is ongoing.
    /// </summary>
    struct CollisionStay
    {
        NEON_EXPORT_FLECS(CollisionStay, "Collision Stay")
        {
        }

        mutable CollisionCallback Callback;
    };

    /// <summary>
    /// Called when a collision ends.
    /// </summary>
    struct CollisionExit
    {
        NEON_EXPORT_FLECS(CollisionExit, "Collision Exit")
        {
        }

        mutable CollisionCallback Callback;
    };

    /// <summary>
    /// Called when a trigger is entered.
    /// </summary>
    struct TriggerEnter
    {
        NEON_EXPORT_FLECS(TriggerEnter, "Trigger Enter")
        {
        }

        mutable CollisionCallback Callback;
    };

    /// <summary>
    /// Called when a trigger is ongoing.
    /// </summary>
    struct TriggerStay
    {
        NEON_EXPORT_FLECS(TriggerStay, "Trigger Stay")
        {
        }

        mutable CollisionCallback Callback;
    };

    /// <summary>
    /// Called when a trigger ends.
    /// </summary>
    struct TriggerExit
    {
        NEON_EXPORT_FLECS(TriggerExit, "Trigger Exit")
        {
        }

        mutable CollisionCallback Callback;
    };

    //

    struct CollisionShape
    {
        NEON_EXPORT_FLECS_COMPONENT(CollisionShape, "Collision Shape")
        {
        }

        UPtr<btCollisionShape> BulletShape = nullptr;
    };

    struct CollisionObject
    {
        NEON_EXPORT_FLECS_COMPONENT(CollisionObject, "Collision Object")
        {
        }

        static constexpr auto MaskAll = std::numeric_limits<uint32_t>::max();

        UPtr<btCollisionObject> BulletObject = nullptr;

        uint32_t Group = 1;
        uint32_t Mask  = MaskAll;

        /// <summary>
        /// Create a new collision object as static body.
        /// </summary>
        static btCollisionObject* AddStaticBody(
            flecs::entity& Target,
            uint32_t       Group = 1,
            uint32_t       Mask  = MaskAll);

        /// <summary>
        /// Create a new collision object as rigid body.
        /// </summary>
        static btCollisionObject* AddRigidBody(
            flecs::entity& Target,
            float          Mass,
            uint32_t       Group = 1,
            uint32_t       Mask  = MaskAll);

        /// <summary>
        /// Create a new collision object as kinematic body.
        /// </summary>
        static btCollisionObject* AddKinematicBody(
            flecs::entity& Target,
            float          Mass,
            uint32_t       Group = 1,
            uint32_t       Mask  = MaskAll);

        /// <summary>
        /// Get the collision object as rigid body.
        /// </summary>
        [[nodiscard]] btRigidBody* AsRigidBody() const;

        /// <summary>
        /// Get the collision object as soft body.
        /// </summary>
        [[nodiscard]] btSoftBody* AsSoftBody() const;
    };
} // namespace Neon::Scene::Component