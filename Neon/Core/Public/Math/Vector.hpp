#pragma once
#include <Math/MVector.hpp>

namespace Neon
{
    class Matrix4x4;
    class Matrix3x3;

    class Vector2D : public Neon::MVector<float, 2>
    {
    public:
        using MVector::MVector;

        constexpr Vector2D(float X, float Y) noexcept :
            MVector({ X, Y })
        {
        }

        static const Vector2D One;
        static const Vector2D Zero;

        MVECTOR_IMPL_MATHVEC_DECLARATIONS(Vector2D);

        MVECTOR_IMPL_ACCESSOR(0, x);
        MVECTOR_IMPL_ACCESSOR(1, y);

        [[nodiscard]] float Cross(const Vector2D& Other) const noexcept;
    };

    class Vector2DI : public Neon::MVector<int, 2>
    {
    public:
        using MVector::MVector;

        constexpr Vector2DI(int X, int Y) noexcept :
            MVector({ X, Y })
        {
        }

        static const Vector2DI One;
        static const Vector2DI Zero;

        MVECTOR_IMPL_ACCESSOR(0, x);
        MVECTOR_IMPL_ACCESSOR(1, y);
    };

    class Vector3D : public Neon::MVector<float, 3>
    {
    public:
        using MVector::MVector;

        Vector3D(float X, float Y, float Z) noexcept :
            MVector({ X, Y, Z })
        {
        }

        static const Vector3D One;
        static const Vector3D Zero;

        static const Vector3D Up;
        static const Vector3D Down;

        static const Vector3D Left;
        static const Vector3D Right;

        static const Vector3D Forward;
        static const Vector3D Backward;

        MVECTOR_IMPL_MATHVEC_DECLARATIONS(Vector3D);

        MVECTOR_IMPL_ACCESSOR(0, x);
        MVECTOR_IMPL_ACCESSOR(1, y);
        MVECTOR_IMPL_ACCESSOR(2, z);

        MVECTOR_IMPL_ACCESSOR(0, r);
        MVECTOR_IMPL_ACCESSOR(1, g);
        MVECTOR_IMPL_ACCESSOR(2, b);

        [[nodiscard]] Vector3D Cross(const Vector3D& Other) const noexcept;
    };

    class Vector3DI : public Neon::MVector<int, 3>
    {
    public:
        using MVector::MVector;

        Vector3DI(int X, int Y, int Z) noexcept :
            MVector({ X, Y, Z })
        {
        }

        static const Vector3DI One;
        static const Vector3DI Zero;

        MVECTOR_IMPL_MATHVEC_DECLARATIONS(Vector3DI);

        MVECTOR_IMPL_ACCESSOR(0, x);
        MVECTOR_IMPL_ACCESSOR(1, y);
        MVECTOR_IMPL_ACCESSOR(2, z);

        MVECTOR_IMPL_ACCESSOR(0, r);
        MVECTOR_IMPL_ACCESSOR(1, g);
        MVECTOR_IMPL_ACCESSOR(2, b);
    };

    class Vector4D : public Neon::MVector<float, 4>
    {
    public:
        using MVector::MVector;

        Vector4D(float X, float Y, float Z, float W) noexcept :
            MVector({ X, Y, Z, W })
        {
        }

        static const Vector4D One;
        static const Vector4D Zero;

        MVECTOR_IMPL_MATHVEC_DECLARATIONS(Vector4D);

        MVECTOR_IMPL_ACCESSOR(0, x);
        MVECTOR_IMPL_ACCESSOR(1, y);
        MVECTOR_IMPL_ACCESSOR(2, z);
        MVECTOR_IMPL_ACCESSOR(3, w);

        MVECTOR_IMPL_ACCESSOR(0, r);
        MVECTOR_IMPL_ACCESSOR(1, g);
        MVECTOR_IMPL_ACCESSOR(2, b);
        MVECTOR_IMPL_ACCESSOR(3, a);

        [[nodiscard]] Vector4D Cross(const Vector4D& Other1, const Vector4D& Other2) const noexcept;
    };

    class Vector4DI : public Neon::MVector<int, 4>
    {
    public:
        using MVector::MVector;

        Vector4DI(int X, int Y, int Z, int W) noexcept :
            MVector({ X, Y, Z, W })
        {
        }

        static const Vector4DI One;
        static const Vector4DI Zero;

        MVECTOR_IMPL_MATHVEC_DECLARATIONS(Vector4DI);

        MVECTOR_IMPL_ACCESSOR(0, x);
        MVECTOR_IMPL_ACCESSOR(1, y);
        MVECTOR_IMPL_ACCESSOR(2, z);
        MVECTOR_IMPL_ACCESSOR(3, w);

        MVECTOR_IMPL_ACCESSOR(0, r);
        MVECTOR_IMPL_ACCESSOR(1, g);
        MVECTOR_IMPL_ACCESSOR(2, b);
        MVECTOR_IMPL_ACCESSOR(3, a);
    };

    class Quaternion : public Vector4D
    {
    public:
        [[nodiscard]] static Quaternion RotationRollPitchYaw(const Vector3D& Vec);

        [[nodiscard]] static Quaternion RotationAxis(const Vector3D& Axis, float Angle);

        [[nodiscard]] static Quaternion RotationNormal(const Vector3D& Axis, float Angle);

        [[nodiscard]] static Quaternion RotationMatrix(const Matrix4x4& Mat);

        [[nodiscard]] static Quaternion RotationMatrix(const Matrix3x3& Mat);

        [[nodiscard]] static Quaternion Slerp(const Quaternion& Begin, const Quaternion& End, float T);

    public:
        using Vector4D::Vector4D;

        static const Quaternion Identity;

        MVECTOR_IMPL_ACCESSOR(0, x);
        MVECTOR_IMPL_ACCESSOR(1, y);
        MVECTOR_IMPL_ACCESSOR(2, z);
        MVECTOR_IMPL_ACCESSOR(3, w);

    public:
        void Normalize();

        void GetAxisAngle(Vector3D& Axis, float& Angle) const;

        void Conjugate();

        [[nodiscard]] Vector3D Rotate(const Vector3D& Vec) const;

        MVECTOR_IMPL_MATH_OP(Quaternion, +);
        MVECTOR_IMPL_MATH_OP(Quaternion, -);
        MVECTOR_IMPL_MATH_OP(Quaternion, /);
        MVECTOR_IMPL_MATH_OP(Quaternion, *);
    };

    inline constexpr Vector2D Vector2D::One  = { 1.f, 1.f };
    inline constexpr Vector2D Vector2D::Zero = {};

    inline constexpr Vector2DI Vector2DI::One  = { 1, 1 };
    inline constexpr Vector2DI Vector2DI::Zero = {};

    inline constexpr Vector3D Vector3D::One  = { 1.f, 1.f, 1.f };
    inline constexpr Vector3D Vector3D::Zero = {};

    inline constexpr Vector3DI Vector3DI::One  = { 1, 1, 1 };
    inline constexpr Vector3DI Vector3DI::Zero = {};

    inline constexpr Vector3D Vector3D::Up   = { 0.f, 1.f, 0.f };
    inline constexpr Vector3D Vector3D::Down = { 0.f, -1.f, 0.f };

    inline constexpr Vector3D Vector3D::Left  = { -1.f, 0.f, 0.f };
    inline constexpr Vector3D Vector3D::Right = { 1.f, 0.f, 0.f };

    inline constexpr Vector3D Vector3D::Forward  = { 0.f, 0.f, 1.f };
    inline constexpr Vector3D Vector3D::Backward = { 0.f, 0.f, -1.f };

    inline constexpr Vector4D Vector4D::One  = { 1.f, 1.f, 1.f, 1.f };
    inline constexpr Vector4D Vector4D::Zero = {};

    inline constexpr Vector4DI Vector4DI::One  = { 1, 1, 1, 1 };
    inline constexpr Vector4DI Vector4DI::Zero = {};

    inline constexpr Quaternion Quaternion::Identity = { 0.f, 0.f, 0.f, 1.f };
} // namespace Neon
