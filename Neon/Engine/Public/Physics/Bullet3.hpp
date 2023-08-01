#pragma once

#include <Math/Vector.hpp>
#include <Math/Transform.hpp>
#include <Math/Matrix.hpp>

#include <Bullet3/LinearMath/btVector3.h>
#include <Bullet3/LinearMath/btMatrix3x3.h>
#include <Bullet3/LinearMath/btTransform.h>

namespace Neon::Physics
{
    /// <summary>
    /// Convert Neon::Vector3 to btVector3.
    /// </summary>
    [[nodiscard]] inline btVector3 ToBullet3(
        const Vector3& Vec) noexcept
    {
        return btVector3(Vec.x, Vec.y, -Vec.z);
    }

    /// <summary>
    /// Convert btVector3 to Neon::Vector3.
    /// </summary>
    [[nodiscard]] inline Vector3 FromBullet3(
        const btVector3& Vec) noexcept
    {
        return Vector3(Vec.x(), Vec.y(), -Vec.z());
    }

    //

    /// <summary>
    /// Convert Neon::Vector4 to btVector4.
    /// </summary>
    [[nodiscard]] inline btVector4 ToBullet3(
        const Vector4& Vec) noexcept
    {
        return btVector4(Vec.x, Vec.y, -Vec.z, Vec.w);
    }

    /// <summary>
    /// Convert btVector4 to Neon::Vector4.
    /// </summary>
    [[nodiscard]] inline Vector4 FromBullet3(
        const btVector4& Vec) noexcept
    {
        return Vector4(Vec.x(), Vec.y(), -Vec.z(), Vec.w());
    }

    //

    /// <summary>
    /// Convert Neon::Matrix3x3 to btMatrix3x3.
    /// </summary>
    [[nodiscard]] inline btMatrix3x3 ToBullet3(
        const Matrix3x3& Mat) noexcept
    {
        return btMatrix3x3(
            Mat[0].x, Mat[0].y, Mat[0].z,
            Mat[1].x, Mat[1].y, Mat[1].z,
            Mat[2].x, Mat[2].y, Mat[2].z);
    }

    /// <summary>
    /// Convert btVector4 to Neon::Vector4.
    /// </summary>
    [[nodiscard]] inline Matrix3x3 FromBullet3(
        const btMatrix3x3& Mat) noexcept
    {
        return Matrix3x3(
            Mat[0].x(), Mat[0].y(), Mat[0].z(),
            Mat[1].x(), Mat[1].y(), Mat[1].z(),
            Mat[2].x(), Mat[2].y(), Mat[2].z());
    }

    //

    /// <summary>
    /// Convert Neon::Transform to btTransform.
    /// </summary>
    [[nodiscard]] inline btTransform ToBullet3(
        const TransformMatrix& Transform) noexcept
    {
        btTransform Result;
        Result.setBasis(ToBullet3(Transform.GetBasis()));
        Result.setOrigin(ToBullet3(Transform.GetPosition()));
        return Result;
    }

    /// <summary>
    /// Convert btTransform to Neon::Transform.
    /// </summary>
    [[nodiscard]] inline TransformMatrix FromBullet3(
        const btTransform& Transform) noexcept
    {
        return TransformMatrix(
            FromBullet3(Transform.getBasis()),
            FromBullet3(Transform.getOrigin()));
    }
} // namespace Neon::Physics