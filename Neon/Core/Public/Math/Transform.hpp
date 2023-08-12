#pragma once

#include <Math/Vector.hpp>
#include <Math/Matrix.hpp>

namespace Neon
{
    class TransformMatrix
    {
    public:
        TransformMatrix(
            const Matrix3x3& Basis    = Mat::Identity<Matrix3x3>,
            const Vector3&   Position = Vec::Zero<Vector3>) :
            m_Basis(Basis),
            m_Position(Position)
        {
        }

    public:
        /// <summary>
        /// Get basis of transform
        /// </summary>
        [[nodiscard]] const Matrix3x3& GetBasis() const noexcept;

        /// <summary>
        /// Get basis of transform
        /// </summary>
        [[nodiscard]] Matrix3x3& GetBasis() noexcept;

        /// <summary>
        /// Set basis of transform
        /// </summary>
        void SetBasis(
            const Matrix3x3& Basis) noexcept;

    public:
        /// <summary>
        /// Get rotation of transform
        /// </summary>
        [[nodiscard]] Quaternion GetRotation() const noexcept;

        /// <summary>
        /// Set rotation of transform
        /// </summary>
        void SetRotation(
            const Quaternion& Rotation) noexcept;

        /// <summary>
        /// Accumulate rotation of transform
        /// </summary>
        void AppendRotation(
            const Quaternion& Rotation) noexcept;

    public:
        /// <summary>
        /// Get rotation of transform
        /// </summary>
        [[nodiscard]] Vector3 GetRotationEuler() const noexcept;

        /// <summary>
        /// Set rotation of transform
        /// </summary>
        void SetRotationEuler(
            const Vector3& Rotation) noexcept;

        /// <summary>
        /// Accumulate rotation of transform
        /// </summary>
        void AppendRotationEuler(
            const Vector3& Rotation) noexcept;

    public:
        /// <summary>
        /// Get scale of transform
        /// </summary>
        [[nodiscard]] Vector3 GetScale() const noexcept;

        /// <summary>
        /// Set scale of transform
        /// </summary>
        void SetScale(
            const Vector3& Scale) noexcept;

        /// <summary>
        /// Accumulate scale of transform
        /// </summary>
        void AppendScale(
            const Vector3& Scale) noexcept;

    public:
        /// <summary>
        /// Get position of transform
        /// </summary>
        [[nodiscard]] const Vector3& GetPosition() const noexcept;

        /// <summary>
        /// Get position of transform
        /// </summary>
        [[nodiscard]] Vector3& GetPosition() noexcept;

        /// <summary>
        /// Set position of transform
        /// </summary>
        void SetPosition(
            const Vector3& Position) noexcept;

    public:
        /// <summary>
        /// Convert transform to matrix
        /// </summary>
        [[nodiscard]] Matrix4x4 ToMat4x4() const noexcept;

        /// <summary>
        /// Convert transform to matrix
        /// </summary>
        [[nodiscard]] Matrix4x4 ToMat4x4Transposed() const noexcept;

        [[nodiscard]] TransformMatrix operator*(
            const TransformMatrix& Other) const noexcept;

        [[nodiscard]] TransformMatrix& operator*=(
            const TransformMatrix& Other) noexcept;

    private:
        Matrix3x3 m_Basis;
        Vector3   m_Position;
    };
} // namespace Neon
