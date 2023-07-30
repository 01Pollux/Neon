#pragma once

#include <flecs/flecs.h>
#include <Scene/Component/Component.hpp>

#include <Bullet3/btBulletCollisionCommon.h>

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

namespace Neon::Scene::Component
{
    struct CollisionShape
    {
        btCollisionShape* BulletShape = nullptr;
    };

    struct CollisionObject
    {
        static constexpr auto MaskAll = std::numeric_limits<uint32_t>::max();

        NEON_EXPORT_COMPONENT();

        btCollisionObject* BulletObject = nullptr;

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