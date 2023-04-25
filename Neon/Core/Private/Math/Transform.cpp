#include <CorePCH.hpp>
#include <Math/Transform.hpp>

namespace Neon
{
    const Matrix3x3& TransformMatrix::GetBasis() const noexcept
    {
        return m_Basis;
    }

    const Vector3D& TransformMatrix::GetPosition() const noexcept
    {
        return m_Position;
    }

    void TransformMatrix::SetBasis(
        const Matrix3x3& Basis) noexcept
    {
        m_Basis = Basis;
    }

    void TransformMatrix::SetPosition(
        const Vector3D& Position) noexcept
    {
        m_Position = Position;
    }

    Matrix4x4 TransformMatrix::ToMat4x4() const noexcept
    {
        Matrix4x4 Mat = m_Basis;
        Mat.SetPosition(m_Position);
        return Mat;
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
