#include <CorePCH.hpp>
#include <Math/Transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Neon
{
    const Matrix3x3& TransformMatrix::GetBasis() const noexcept
    {
        return m_Basis;
    }

    Matrix3x3& TransformMatrix::GetBasis() noexcept
    {
        return m_Basis;
    }

    void TransformMatrix::SetBasis(
        const Matrix3x3& Basis) noexcept
    {
        m_Basis = Basis;
    }

    //

    Quaternion TransformMatrix::GetRotation() const noexcept
    {
        return glm::quat_cast(m_Basis);
    }

    void TransformMatrix::SetRotation(
        const Quaternion& Rotation) noexcept
    {
        m_Basis = glm::mat3_cast(Rotation);
    }

    void TransformMatrix::AppendRotation(
        const Quaternion& Rotation) noexcept
    {
        m_Basis = glm::mat3_cast(Rotation) * m_Basis;
    }

    //

    Vector3 TransformMatrix::GetRotationEuler() const noexcept
    {
        return glm::eulerAngles(glm::quat_cast(m_Basis));
    }

    void TransformMatrix::SetRotationEuler(const Vector3& Rotation) noexcept
    {
        m_Basis = glm::mat3_cast(Quaternion(Rotation));
    }

    void TransformMatrix::AppendRotationEuler(const Vector3& Rotation) noexcept
    {
        m_Basis = glm::mat3_cast(Quaternion(Rotation)) * m_Basis;
    }

    //

    Vector3 TransformMatrix::GetScale() const noexcept
    {
        return Vector3(
            glm::length(m_Basis[0]),
            glm::length(m_Basis[1]),
            glm::length(m_Basis[2]));
    }

    void TransformMatrix::SetScale(
        const Vector3& Scale) noexcept
    {
        for (int i = 0; i < 3; ++i)
        {
            m_Basis[i] = (m_Basis[i] / glm::length(m_Basis[i])) * Scale[i];
        }
    }

    void TransformMatrix::AppendScale(
        const Vector3& Scale) noexcept
    {
        for (int i = 0; i < 3; ++i)
        {
            m_Basis[i] *= Scale[i];
        }
    }

    //

    const Vector3& TransformMatrix::GetPosition() const noexcept
    {
        return m_Position;
    }

    Vector3& TransformMatrix::GetPosition() noexcept
    {
        return m_Position;
    }

    void TransformMatrix::SetPosition(
        const Vector3& Position) noexcept
    {
        m_Position = Position;
    }

    //

    Matrix4x4 TransformMatrix::ToMat4x4() const noexcept
    {
        return {
            Vector4(m_Basis[0], 0.f),
            Vector4(m_Basis[1], 0.f),
            Vector4(m_Basis[2], 0.f),
            Vector4(m_Position, 1.f)
        };
    }

    Matrix4x4 TransformMatrix::ToMat4x4Transposed() const noexcept
    {
        return Matrix4x4(
            m_Basis[0][0], m_Basis[1][0], m_Basis[2][0], m_Position.x,
            m_Basis[0][1], m_Basis[1][1], m_Basis[2][1], m_Position.y,
            m_Basis[0][2], m_Basis[1][2], m_Basis[2][2], m_Position.z,
            0.f, 0.f, 0.f, 1.f);
    }

    TransformMatrix TransformMatrix::operator*(
        const TransformMatrix& Other) const noexcept
    {
        return TransformMatrix(GetBasis() * Other.GetBasis(), GetPosition() + Other.GetPosition());
    }

    TransformMatrix& TransformMatrix::operator*=(
        const TransformMatrix& Other) noexcept
    {
        m_Basis *= Other.GetBasis();
        m_Position += Other.GetPosition();
        return *this;
    }
} // namespace Neon
