#pragma once

#include <flecs/flecs.h>
#include <Scene/Component/Component.hpp>
#include <Physics/Bullet3.hpp>

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
        NEON_EXPORT_COMPONENT();
        mutable CollisionCallback Callback;
    };

    /// <summary>
    /// Called when a collision is ongoing.
    /// </summary>
    struct CollisionStay
    {
        NEON_EXPORT_COMPONENT();
        mutable CollisionCallback Callback;
    };

    /// <summary>
    /// Called when a collision ends.
    /// </summary>
    struct CollisionExit
    {
        NEON_EXPORT_COMPONENT();
        mutable CollisionCallback Callback;
    };

    /// <summary>
    /// Called when a trigger is entered.
    /// </summary>
    struct TriggerEnter
    {
        NEON_EXPORT_COMPONENT();
        mutable CollisionCallback Callback;
    };

    /// <summary>
    /// Called when a trigger is ongoing.
    /// </summary>
    struct TriggerStay
    {
        NEON_EXPORT_COMPONENT();
        mutable CollisionCallback Callback;
    };

    /// <summary>
    /// Called when a trigger ends.
    /// </summary>
    struct TriggerExit
    {
        NEON_EXPORT_COMPONENT();
        mutable CollisionCallback Callback;
    };

    //

    struct CollisionShape
    {
        NEON_EXPORT_COMPONENT();
        UPtr<btCollisionShape> BulletShape = nullptr;
    };

    struct CollisionObject
    {
        NEON_EXPORT_COMPONENT();
        static constexpr auto MaskAll = std::numeric_limits<uint32_t>::max();

        UPtr<btCollisionObject> BulletObject = nullptr;

        uint32_t Group = 1;
        uint32_t Mask  = MaskAll;

        /// <summary>
        /// Create a new collision object as static body.
        /// </summary>
        static Actor AddStaticBody(
            Actor    Target,
            uint32_t Group = 1,
            uint32_t Mask  = MaskAll);

        /// <summary>
        /// Create a new collision object as rigid body.
        /// </summary>
        static Actor AddRigidBody(
            Actor    Target,
            float    Mass,
            uint32_t Group = 1,
            uint32_t Mask  = MaskAll);

        /// <summary>
        /// Create a new collision object as kinematic body.
        /// </summary>
        static Actor AddKinematicBody(
            Actor    Target,
            float    Mass,
            uint32_t Group = 1,
            uint32_t Mask  = MaskAll);
    };
} // namespace Neon::Scene::Component