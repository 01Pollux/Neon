#pragma once

#include <Math/Vector.hpp>

namespace Neon
{
    class Matrix3x3;

    class [[nodiscard]] Matrix4x4
    {
    public:
        static const Matrix4x4 Zero;
        static const Matrix4x4 Identity;

        Matrix4x4() = default;
        Matrix4x4(const Matrix3x3& Mat3);

        [[nodiscard]] static Matrix4x4 Translation(
            const Vector3D& Vec);

        [[nodiscard]] static Matrix4x4 Scale(
            const Vector3D& Vec);

        [[nodiscard]] static Matrix4x4 RotationX(
            float Angle);

        [[nodiscard]] static Matrix4x4 RotationY(
            float Angle);

        [[nodiscard]] static Matrix4x4 RotationZ(
            float Angle);

        [[nodiscard]] static Matrix4x4 RotationRollPitchYaw(
            const Vector3D& Vec);

        [[nodiscard]] static Matrix4x4 RotationQuaternion(
            const Quaternion& Quat);

        [[nodiscard]] static Matrix4x4 RotationNormal(
            const Vector3D& Axis,
            float           Angle);

        [[nodiscard]] static Matrix4x4 RotationAxis(
            const Vector3D& Axis,
            float           Angle);

        [[nodiscard]] static Matrix4x4 LookAt(
            const Vector3D& EyePosition,
            const Vector3D& FocusPosition,
            const Vector3D& UpDirection);

        [[nodiscard]] static Matrix4x4 LookTo(
            const Vector3D& EyePosition,
            const Vector3D& EyeDirection,
            const Vector3D& UpDirection);

        [[nodiscard]] static Matrix4x4 Perspective(
            float ViewWidth,
            float ViewHeight,
            float NearZ,
            float FarZ);

        [[nodiscard]] static Matrix4x4 PerspectiveFov(
            float FovAngleY,
            float AspectRatio,
            float NearZ,
            float FarZ);

        [[nodiscard]] static Matrix4x4 Orthographic(
            float ViewWidth,
            float ViewHeight,
            float NearZ,
            float FarZ);

        [[nodiscard]] static Matrix4x4 SRT(
            const Matrix4x4& Scale,
            const Matrix4x4& Rotation,
            const Matrix4x4& Translation);

    public:
        [[nodiscard]] Vector2D DoTransform(
            const Vector2D& Vec) const;
        [[nodiscard]] Vector2D DoTransformCoord(
            const Vector2D& Vec) const;
        [[nodiscard]] Vector2D DoTransformNormal(
            const Vector2D& Vec) const;

        [[nodiscard]] Vector3D DoTransform(
            const Vector3D& Vec) const;
        [[nodiscard]] Vector3D DoTransformCoord(
            const Vector3D& Vec) const;
        [[nodiscard]] Vector3D DoTransformNormal(
            const Vector3D& Vec) const;

        [[nodiscard]] Vector4D DoTransform(
            const Vector4D& Vec) const;

        [[nodiscard]] Matrix4x4 GetInverse() const;

        [[nodiscard]] Matrix4x4 GetTranspose() const;

        [[nodiscard]] Vector4D GetDeterminant() const;

        [[nodiscard]] constexpr float& operator()(
            size_t Row,
            size_t Column) noexcept
        {
            return m_Data[Row][Column];
        }

        [[nodiscard]] constexpr Vector4D& operator()(
            size_t Row) noexcept
        {
            return m_Data[Row];
        }

        [[nodiscard]] constexpr float operator()(
            size_t Row,
            size_t Column) const noexcept
        {
            return m_Data[Row][Column];
        }

        [[nodiscard]] constexpr const Vector4D& operator()(
            size_t Row) const noexcept
        {
            return m_Data[Row];
        }

    public:
        [[nodiscard]] std::tuple<Vector4D, Quaternion, Vector3D> GetPositionRotationScale() const;

        [[nodiscard]] Vector3D GetScale() const;
        void                   SetScale(const Vector3D& Scale);

        [[nodiscard]] Quaternion GetRotationQuat() const;

        [[nodiscard]] Vector3D GetPosition() const;
        void                   SetPosition(
                              const Vector3D& Position)
        {
            SetPosition(Vector4D(Position.x(), Position.y(), Position.z(), 1.f));
        }
        void SetPosition(
            const Vector4D& Position);

        Matrix4x4 operator*(
            const Matrix4x4& Other) const;
        Matrix4x4& operator*=(
            const Matrix4x4& Other);

    private:
        [[nodiscard]] static constexpr Matrix4x4 _Identity()
        {
            Matrix4x4 Mat;
            Mat(0, 0) = Mat(1, 1) = Mat(2, 2) = Mat(3, 3) = 1.f;
            return Mat;
        }

    private:
        std::array<Vector4D, 4> m_Data;
    };

    class [[nodiscard]] Matrix3x3
    {
    public:
        static const Matrix3x3 Zero;
        static const Matrix3x3 Identity;

        Matrix3x3() = default;
        Matrix3x3(const Matrix4x4& Mat4);

        [[nodiscard]] static Matrix3x3 Scale(
            const Vector3D& Vec);

        [[nodiscard]] static Matrix3x3 RotationX(
            float Angle);

        [[nodiscard]] static Matrix3x3 RotationY(
            float Angle);

        [[nodiscard]] static Matrix3x3 RotationZ(
            float Angle);

        [[nodiscard]] static Matrix3x3 RotationRollPitchYaw(
            const Vector3D& Vec);

        [[nodiscard]] static Matrix3x3 RotationQuaternion(
            const Quaternion& Quat);

        [[nodiscard]] static Matrix3x3 RotationNormal(
            const Vector3D& Axis,
            float           Angle);

        [[nodiscard]] static Matrix3x3 RotationAxis(
            const Vector3D& Axis,
            float           Angle);

        [[nodiscard]] static Matrix3x3 SR(
            const Matrix3x3& Scale,
            const Matrix3x3& Rotation);

    public:
        [[nodiscard]] Matrix3x3 GetInverse() const;

        [[nodiscard]] Matrix3x3 GetTranspose() const;

        [[nodiscard]] Vector4D GetDeterminant() const;

        [[nodiscard]] constexpr float& operator()(
            size_t Row,
            size_t Column) noexcept
        {
            return m_Data[Row][Column];
        }

        [[nodiscard]] constexpr Vector3D& operator()(
            size_t Row) noexcept
        {
            return m_Data[Row];
        }

        [[nodiscard]] constexpr float operator()(
            size_t Row,
            size_t Column) const noexcept
        {
            return m_Data[Row][Column];
        }

        [[nodiscard]] constexpr const Vector3D& operator()(
            size_t Row) const noexcept
        {
            return m_Data[Row];
        }

    public:
        [[nodiscard]] std::tuple<Quaternion, Vector3D> GetRotationScale() const;

        [[nodiscard]] Vector3D GetScale() const;
        void                   SetScale(
                              const Vector3D& Scale);

        [[nodiscard]] Quaternion GetRotationQuat() const;

        Matrix3x3 operator*(
            const Matrix3x3& Other) const;
        Matrix3x3& operator*=(
            const Matrix3x3& Other);

    private:
        [[nodiscard]] static constexpr Matrix3x3 _Identity()
        {
            Matrix3x3 Mat;
            Mat(0, 0) = Mat(1, 1) = Mat(2, 2) = 1.f;
            return Mat;
        }

    private:
        std::array<Vector3D, 3> m_Data;
    };

    inline constexpr Matrix4x4 Matrix4x4::Zero     = {};
    inline constexpr Matrix4x4 Matrix4x4::Identity = _Identity();

    inline constexpr Matrix3x3 Matrix3x3::Zero     = {};
    inline constexpr Matrix3x3 Matrix3x3::Identity = _Identity();
} // namespace Neon
