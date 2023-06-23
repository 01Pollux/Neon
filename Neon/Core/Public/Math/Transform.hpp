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

        /// <summary>
        /// Get basis of transform
        /// </summary>
        [[nodiscard]] const Matrix3x3& GetBasis() const noexcept;

        /// <summary>
        /// Get position of transform
        /// </summary>
        [[nodiscard]] const Vector3& GetPosition() const noexcept;

        /// <summary>
        /// Set basis of transform
        /// </summary>
        void SetBasis(
            const Matrix3x3& Basis) noexcept;

        /// <summary>
        /// Set position of transform
        /// </summary>
        void SetPosition(
            const Vector3& Position) noexcept;

        /// <summary>
        /// Convert transform to matrix
        /// </summary>
        [[nodiscard]] Matrix4x4 ToMat4x4() const noexcept;

        [[nodiscard]] TransformMatrix operator*(
            const TransformMatrix& Other) const noexcept;

        [[nodiscard]] TransformMatrix& operator*=(
            const TransformMatrix& Other) noexcept;

    private:
        Matrix3x3 m_Basis;
        Vector3   m_Position;
    };
} // namespace Neon
