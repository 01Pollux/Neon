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

    const Vector3& TransformMatrix::GetPosition() const noexcept
    {
        return m_Position;
    }

    Vector3& TransformMatrix::GetPosition() noexcept
    {
        return m_Position;
    }

    void TransformMatrix::SetBasis(
        const Matrix3x3& Basis) noexcept
    {
        m_Basis = Basis;
    }

    void TransformMatrix::SetPosition(
        const Vector3& Position) noexcept
    {
        m_Position = Position;
    }

    Matrix4x4 TransformMatrix::ToMat4x4() const noexcept
    {
        return {
            Vector4(m_Basis[0], 0.f),
            Vector4(m_Basis[1], 0.f),
            Vector4(m_Basis[2], 0.f),
            Vector4(m_Position, 1.f)
        };
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
