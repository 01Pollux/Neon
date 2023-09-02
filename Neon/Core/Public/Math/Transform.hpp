#pragma once

#include <Math/Vector.hpp>
#include <Math/Matrix.hpp>
#include <glm/gtx/matrix_interpolation.hpp>

namespace Neon
{
    template<bool _IsAffine>
    class TransformMatrixT
    {
    public:
        TransformMatrixT(
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
        [[nodiscard]] Matrix3x3 GetBasisNormalized() const noexcept
        {
            auto Basis = m_Basis;
            Basis[0]   = glm::normalize(Basis[0]);
            Basis[1]   = glm::normalize(Basis[1]);
            Basis[2]   = glm::normalize(Basis[2]);
            return Basis;
        }

        /// <summary>
        /// Get basis of transform
        /// </summary>
        [[nodiscard]] const Matrix3x3& GetBasis() const noexcept
        {
            return m_Basis;
        }

        /// <summary>
        /// Get basis of transform
        /// </summary>
        [[nodiscard]] Matrix3x3& GetBasis() noexcept
        {
            return m_Basis;
        }

        /// <summary>
        /// Set basis of transform
        /// </summary>
        void SetBasis(
            const Matrix3x3& Basis) noexcept
        {
            m_Basis = Basis;
        }

    public:
        /// <summary>
        /// Get rotation of transform
        /// </summary>
        [[nodiscard]] Quaternion GetRotation() const noexcept
        {
            return glm::quat_cast(m_Basis);
        }

        /// <summary>
        /// Set rotation of transform
        /// </summary>
        void SetRotation(
            const Quaternion& Rotation) noexcept
        {
            if constexpr (_IsAffine)
            {
                auto Scale = GetScale();
                m_Basis    = glm::mat3_cast(Rotation);
                SetScale(Scale);
            }
            else
            {
                m_Basis = glm::mat3_cast(Rotation);
            }
        }

        /// <summary>
        /// Accumulate rotation of transform
        /// </summary>
        void AppendRotation(
            const Quaternion& Rotation) noexcept
        {
            SetRotation(GetRotation() * Rotation);
        }

    public:
        /// <summary>
        /// Get rotation of transform
        /// </summary>
        [[nodiscard]] Vector3 GetRotationEuler() const noexcept
        {
            if constexpr (_IsAffine)
            {
                return glm::eulerAngles(glm::quat_cast(GetBasisNormalized()));
            }
            else
            {
                return glm::eulerAngles(glm::quat_cast(GetBasis()));
            }
        }

        /// <summary>
        /// Set rotation of transform
        /// </summary>
        void SetRotationEuler(
            const Vector3& Rotation) noexcept
        {
            SetRotation(Quaternion(Rotation));
        }

        /// <summary>
        /// Accumulate rotation of transform
        /// </summary>
        void AppendRotationEuler(
            const Vector3& Rotation) noexcept
        {
            SetRotation(GetRotation() * Quaternion(Rotation));
        }

    public:
        /// <summary>
        /// Get axis angle of transform
        /// </summary>
        [[nodiscard]] float GetAxisAngle(
            Vector3& Axis) const noexcept
        {
            float Angle;
            if constexpr (_IsAffine)
            {
                glm::axisAngle(Matrix4x4(GetBasisNormalized()), Axis, Angle);
            }
            else
            {
                glm::axisAngle(Matrix4x4(GetBasis()), Axis, Angle);
            }
            return Angle;
        }

        /// <summary>
        /// Set axis angle of transform
        /// </summary>
        void SetAxisAngle(
            const Vector3& Axis,
            float          Angle) noexcept
        {
            SetRotation(glm::angleAxis(Angle, Axis));
        }

    public:
        /// <summary>
        /// Get scale of transform
        /// </summary>
        template<typename = std::enable_if_t<_IsAffine>>
        [[nodiscard]] Vector3 GetScale() const noexcept
        {
            return {
                glm::length(m_Basis[0]),
                glm::length(m_Basis[1]),
                glm::length(m_Basis[2])
            };
        }

        /// <summary>
        /// Set scale of transform
        /// </summary>
        template<typename = std::enable_if_t<_IsAffine>>
        void SetScale(
            const Vector3& Scale) noexcept
        {
            m_Basis[0] = glm::normalize(m_Basis[0]) * Scale.x;
            m_Basis[1] = glm::normalize(m_Basis[1]) * Scale.y;
            m_Basis[2] = glm::normalize(m_Basis[2]) * Scale.z;
        }

        /// <summary>
        /// Accumulate scale of transform
        /// </summary>
        template<typename = std::enable_if_t<_IsAffine>>
        void AppendScale(
            const Vector3& Scale) noexcept
        {
            SetScale(GetScale() * Scale);
        }

    public:
        /// <summary>
        /// Get position of transform
        /// </summary>
        [[nodiscard]] const Vector3& GetPosition() const noexcept
        {
            return m_Position;
        }

        /// <summary>
        /// Get position of transform
        /// </summary>
        [[nodiscard]] Vector3& GetPosition() noexcept
        {
            return m_Position;
        }

        /// <summary>
        /// Set position of transform
        /// </summary>
        void SetPosition(
            const Vector3& Position) noexcept
        {
            m_Position = Position;
        }

    public:
        /// <summary>
        /// Convert transform to matrix
        /// </summary>
        [[nodiscard]] Matrix4x4 ToMat4x4() const noexcept
        {
            return {
                Vector4(m_Basis[0], 0.f),
                Vector4(m_Basis[1], 0.f),
                Vector4(m_Basis[2], 0.f),
                Vector4(m_Position, 1.f)
            };
        }

        /// <summary>
        /// Convert transform to matrix
        /// </summary>
        [[nodiscard]] Matrix4x4 ToMat4x4Transposed() const noexcept
        {
            return Matrix4x4(
                m_Basis[0][0], m_Basis[1][0], m_Basis[2][0], m_Position.x,
                m_Basis[0][1], m_Basis[1][1], m_Basis[2][1], m_Position.y,
                m_Basis[0][2], m_Basis[1][2], m_Basis[2][2], m_Position.z,
                0.f, 0.f, 0.f, 1.f);
        }

    public:
        [[nodiscard]] TransformMatrixT operator*(
            const TransformMatrixT& Other) const noexcept
        {
            return TransformMatrix(GetBasis() * Other.GetBasis(), GetPosition() + Other.GetPosition());
        }

        TransformMatrixT& operator*=(
            const TransformMatrixT& Other) noexcept
        {
            m_Basis *= Other.m_Basis;
            m_Position += Other.m_Position;
            return *this;
        }

    private:
        friend class boost::serialization::access;
        template<typename _Archive>
        void serialize(
            _Archive& Archive,
            uint32_t)
        {
            Archive& m_Basis& m_Position;
        }

    private:
        Matrix3x3 m_Basis;
        Vector3   m_Position;
    };

    using AffineTransformMatrix = TransformMatrixT<true>;
    using TransformMatrix       = TransformMatrixT<false>;
} // namespace Neon
