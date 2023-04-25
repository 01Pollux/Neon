#include <CorePCH.hpp>
#include <type_traits>

#include <Math/Matrix.hpp>
#include <DirectXMath.h>

using namespace DirectX;

namespace Neon
{
#define PL_IMPL_MATHVEC_DEFINITIONS(Class, Dim, Type, XMType)                                                                                 \
    [[nodiscard]] XMType* To##Type##Dim(Class& Vec)                                                                                           \
    {                                                                                                                                         \
        return std::bit_cast<XMType*>(Vec.data());                                                                                            \
    }                                                                                                                                         \
                                                                                                                                              \
    [[nodiscard]] const XMType* To##Type##Dim(const Class& Vec)                                                                               \
    {                                                                                                                                         \
        return std::bit_cast<const XMType*>(Vec.data());                                                                                      \
    }                                                                                                                                         \
                                                                                                                                              \
    [[nodiscard]] XMVECTOR Load##Type##Dim(Class& Vec)                                                                                        \
    {                                                                                                                                         \
        return XMLoad##Type##Dim(To##Type##Dim(Vec));                                                                                         \
    }                                                                                                                                         \
                                                                                                                                              \
    [[nodiscard]] XMVECTOR Load##Type##Dim(const Class& Vec)                                                                                  \
    {                                                                                                                                         \
        return XMLoad##Type##Dim(To##Type##Dim(Vec));                                                                                         \
    }                                                                                                                                         \
                                                                                                                                              \
    bool Class::IsNan() const noexcept                                                                                                        \
    {                                                                                                                                         \
        XMVECTOR Res = XMVectorSelect(g_XMOne, g_XMZero, XMVectorIsNaN(Neon::Load##Type##Dim(*this)));                                        \
        return XMVectorGetX(Res) == 1;                                                                                                        \
    }                                                                                                                                         \
                                                                                                                                              \
    void Class::Negate() noexcept                                                                                                             \
    {                                                                                                                                         \
        XMVECTOR Res = XMVectorNegate(Neon::Load##Type##Dim(*this));                                                                          \
        XMStore##Type##Dim(Neon::To##Type##Dim(*this), Res);                                                                                  \
    }                                                                                                                                         \
                                                                                                                                              \
    Class::value_type Class::Dot(const Class& Other) const noexcept                                                                           \
    {                                                                                                                                         \
        XMVECTOR Res = XMVector##Dim##Dot(Neon::Load##Type##Dim(*this), Neon::Load##Type##Dim(Other));                                        \
        return XMVectorGetX(Res);                                                                                                             \
    }                                                                                                                                         \
                                                                                                                                              \
    Class::value_type Class::LengthSqr() const noexcept                                                                                       \
    {                                                                                                                                         \
        XMVECTOR Res = XMVector##Dim##LengthSq(Neon::Load##Type##Dim(*this));                                                                 \
        return XMVectorGetX(Res);                                                                                                             \
    }                                                                                                                                         \
                                                                                                                                              \
    Class::value_type Class::Length() const noexcept                                                                                          \
    {                                                                                                                                         \
        XMVECTOR Res = XMVector##Dim##Length(Neon::Load##Type##Dim(*this));                                                                   \
        return XMVectorGetX(Res);                                                                                                             \
    }                                                                                                                                         \
                                                                                                                                              \
    bool Class::IsGreaterThan(const Class& Other) const noexcept                                                                              \
    {                                                                                                                                         \
        return XMVector##Dim##Greater(Neon::Load##Type##Dim(*this), Neon::Load##Type##Dim(Other));                                            \
    }                                                                                                                                         \
                                                                                                                                              \
    bool Class::IsLessThan(const Class& Other) const noexcept                                                                                 \
    {                                                                                                                                         \
        return XMVector##Dim##Less(Neon::Load##Type##Dim(*this), Neon::Load##Type##Dim(Other));                                               \
    }                                                                                                                                         \
                                                                                                                                              \
    void Class::Normalize() noexcept                                                                                                          \
    {                                                                                                                                         \
        XMVECTOR Res = XMVector##Dim##Normalize(Neon::Load##Type##Dim(*this));                                                                \
        XMStore##Type##Dim(Neon::To##Type##Dim(*this), Res);                                                                                  \
    }                                                                                                                                         \
                                                                                                                                              \
    bool Class::NearEqual(const Class& Other, value_type Tolerance) const noexcept                                                            \
    {                                                                                                                                         \
        return XMVector##Dim##NearEqual(Neon::Load##Type##Dim(*this), Neon::Load##Type##Dim(Other), XMVectorReplicate(Tolerance));            \
    }                                                                                                                                         \
                                                                                                                                              \
    bool Class::Within(const Class& Min, const Class& Max) const noexcept                                                                     \
    {                                                                                                                                         \
        XMVECTOR Vec = Neon::Load##Type##Dim(*this);                                                                                          \
        return XMVector##Dim##GreaterOrEqual(Vec, Neon::Load##Type##Dim(Min)) && XMVector##Dim##LessOrEqual(Vec, Neon::Load##Type##Dim(Max)); \
    }                                                                                                                                         \
                                                                                                                                              \
    void Class::MultAdd(const Class& A, const Class& B) noexcept                                                                              \
    {                                                                                                                                         \
        XMVECTOR Vec = XMVectorMultiplyAdd(Neon::Load##Type##Dim(*this), Neon::Load##Type##Dim(A), Neon::Load##Type##Dim(B));                 \
        XMStore##Type##Dim(Neon::To##Type##Dim(*this), Vec);                                                                                  \
    }                                                                                                                                         \
                                                                                                                                              \
    Class& Class::operator=(value_type Val) noexcept                                                                                          \
    {                                                                                                                                         \
        XMStore##Type##Dim(Neon::To##Type##Dim(*this), XMVectorReplicate(Val));                                                               \
        return *this;                                                                                                                         \
    }                                                                                                                                         \
                                                                                                                                              \
    Class Class::operator-() const noexcept                                                                                                   \
    {                                                                                                                                         \
        Class Ret;                                                                                                                            \
        XMStore##Type##Dim(Neon::To##Type##Dim(Ret), XMVectorNegate(Neon::Load##Type##Dim(*this)));                                           \
        return Ret;                                                                                                                           \
    }                                                                                                                                         \
                                                                                                                                              \
    Class Class::Min(const Class& Other) const noexcept                                                                                       \
    {                                                                                                                                         \
        Class    Ret;                                                                                                                         \
        XMVECTOR Vec = XMVectorMin(Neon::Load##Type##Dim(*this), Neon::Load##Type##Dim(Other));                                               \
        XMStore##Type##Dim(Neon::To##Type##Dim(Ret), Vec);                                                                                    \
        return Ret;                                                                                                                           \
    }                                                                                                                                         \
                                                                                                                                              \
    Class Class::Max(const Class& Other) const noexcept                                                                                       \
    {                                                                                                                                         \
        Class    Ret;                                                                                                                         \
        XMVECTOR Vec = XMVectorMax(Neon::Load##Type##Dim(*this), Neon::Load##Type##Dim(Other));                                               \
        XMStore##Type##Dim(Neon::To##Type##Dim(Ret), Vec);                                                                                    \
        return Ret;                                                                                                                           \
    }                                                                                                                                         \
                                                                                                                                              \
    Class& Class::operator+=(const Class& Other) noexcept                                                                                     \
    {                                                                                                                                         \
        XMVECTOR Vec = XMVectorAdd(Neon::Load##Type##Dim(*this), Neon::Load##Type##Dim(Other));                                               \
        XMStore##Type##Dim(Neon::To##Type##Dim(*this), Vec);                                                                                  \
        return *this;                                                                                                                         \
    }                                                                                                                                         \
                                                                                                                                              \
    Class& Class::operator+=(value_type Val) noexcept                                                                                         \
    {                                                                                                                                         \
        XMVECTOR Vec = XMVectorAdd(Neon::Load##Type##Dim(*this), XMVectorReplicate(Val));                                                     \
        XMStore##Type##Dim(Neon::To##Type##Dim(*this), Vec);                                                                                  \
        return *this;                                                                                                                         \
    }                                                                                                                                         \
                                                                                                                                              \
    Class& Class::operator-=(const Class& Other) noexcept                                                                                     \
    {                                                                                                                                         \
        XMVECTOR Vec = XMVectorSubtract(Neon::Load##Type##Dim(*this), Neon::Load##Type##Dim(Other));                                          \
        XMStore##Type##Dim(Neon::To##Type##Dim(*this), Vec);                                                                                  \
        return *this;                                                                                                                         \
    }                                                                                                                                         \
                                                                                                                                              \
    Class& Class::operator-=(value_type Val) noexcept                                                                                         \
    {                                                                                                                                         \
        XMVECTOR Vec = XMVectorSubtract(Neon::Load##Type##Dim(*this), XMVectorReplicate(Val));                                                \
        XMStore##Type##Dim(Neon::To##Type##Dim(*this), Vec);                                                                                  \
        return *this;                                                                                                                         \
    }                                                                                                                                         \
                                                                                                                                              \
    Class& Class::operator*=(const Class& Other) noexcept                                                                                     \
    {                                                                                                                                         \
        XMVECTOR Vec = XMVectorMultiply(Neon::Load##Type##Dim(*this), Neon::Load##Type##Dim(Other));                                          \
        XMStore##Type##Dim(Neon::To##Type##Dim(*this), Vec);                                                                                  \
        return *this;                                                                                                                         \
    }                                                                                                                                         \
                                                                                                                                              \
    Class& Class::operator*=(value_type Val) noexcept                                                                                         \
    {                                                                                                                                         \
        XMVECTOR Vec = XMVectorMultiply(Neon::Load##Type##Dim(*this), XMVectorReplicate(Val));                                                \
        XMStore##Type##Dim(Neon::To##Type##Dim(*this), Vec);                                                                                  \
        return *this;                                                                                                                         \
    }                                                                                                                                         \
                                                                                                                                              \
    Class& Class::operator/=(const Class& Other) noexcept                                                                                     \
    {                                                                                                                                         \
        XMVECTOR Vec = XMVectorDivide(Neon::Load##Type##Dim(*this), Neon::Load##Type##Dim(Other));                                            \
        XMStore##Type##Dim(Neon::To##Type##Dim(*this), Vec);                                                                                  \
        return *this;                                                                                                                         \
    }                                                                                                                                         \
                                                                                                                                              \
    Class& Class::operator/=(value_type Val) noexcept                                                                                         \
    {                                                                                                                                         \
        XMVECTOR Vec = XMVectorDivide(Neon::Load##Type##Dim(*this), XMVectorReplicate(Val));                                                  \
        XMStore##Type##Dim(Neon::To##Type##Dim(*this), Vec);                                                                                  \
        return *this;                                                                                                                         \
    }
    PL_IMPL_MATHVEC_DEFINITIONS(Vector2D, 2, Float, XMFLOAT2);
    PL_IMPL_MATHVEC_DEFINITIONS(Vector3D, 3, Float, XMFLOAT3);
    PL_IMPL_MATHVEC_DEFINITIONS(Vector4D, 4, Float, XMFLOAT4);

    float Vector2D::Cross(const Vector2D& Other) const noexcept
    {
        XMVECTOR Res = XMVector2Cross(LoadFloat2(*this), LoadFloat2(Other));
        return XMVectorGetX(Res);
    }

    Vector3D Vector3D::Cross(const Vector3D& Other) const noexcept
    {
        Vector3D Vec;
        XMVECTOR Res = XMVector3Cross(LoadFloat3(*this), LoadFloat3(Other));
        XMStoreFloat3(ToFloat3(Vec), Res);
        return Vec;
    }

    Vector4D Vector4D::Cross(const Vector4D& Other1, const Vector4D& Other2) const noexcept
    {
        Vector4D Vec;
        XMVECTOR Res = XMVector4Cross(LoadFloat4(*this), LoadFloat4(Other1), LoadFloat4(Other2));
        XMStoreFloat4(ToFloat4(Vec), Res);
        return Vec;
    }

    //

    [[nodiscard]] XMFLOAT4* ToFloat4(Quaternion& Quat)
    {
        return std::bit_cast<XMFLOAT4*>(Quat.data());
    }

    [[nodiscard]] const XMFLOAT4* ToFloat4(const Quaternion& Quat)
    {
        return std::bit_cast<const XMFLOAT4*>(Quat.data());
    }

    [[nodiscard]] XMVECTOR LoadFloat4(Quaternion& Quat)
    {
        return XMLoadFloat4(ToFloat4(Quat));
    }

    [[nodiscard]] XMVECTOR LoadFloat4(const Quaternion& Quat)
    {
        return XMLoadFloat4(ToFloat4(Quat));
    }

    extern XMMATRIX LoadFloat4x4(const Matrix4x4& Mat);
    extern XMMATRIX LoadFloat3x3(const Matrix3x3& Mat);

    //

    Quaternion Quaternion::RotationRollPitchYaw(const Vector3D& Vec)
    {
        Quaternion Quat;
        XMStoreFloat4(ToFloat4(Quat), XMQuaternionRotationRollPitchYaw(Vec.x(), Vec.y(), Vec.z()));
        return Quat;
    }

    Quaternion Quaternion::RotationAxis(const Vector3D& Axis, float Angle)
    {
        Quaternion Quat;
        XMStoreFloat4(ToFloat4(Quat), XMQuaternionRotationAxis(LoadFloat3(Axis), Angle));
        return Quat;
    }

    Quaternion Quaternion::RotationNormal(const Vector3D& Axis, float Angle)
    {
        Quaternion Quat;
        XMStoreFloat4(ToFloat4(Quat), XMQuaternionRotationNormal(LoadFloat3(Axis), Angle));
        return Quat;
    }

    Quaternion Quaternion::RotationMatrix(const Matrix4x4& Mat)
    {
        Quaternion Quat;
        XMStoreFloat4(ToFloat4(Quat), XMQuaternionRotationMatrix(LoadFloat4x4(Mat)));
        return Quat;
    }

    Quaternion Quaternion::RotationMatrix(const Matrix3x3& Mat)
    {
        Quaternion Quat;
        XMStoreFloat4(ToFloat4(Quat), XMQuaternionRotationMatrix(LoadFloat3x3(Mat)));
        return Quat;
    }

    Quaternion Quaternion::Slerp(const Quaternion& Begin, const Quaternion& End, float T)
    {
        Quaternion Quat;
        XMStoreFloat4(ToFloat4(Quat), XMQuaternionSlerp(LoadFloat4(Begin), LoadFloat4(End), T));
        return Quat;
    }

    void Quaternion::Normalize()
    {
        XMStoreFloat4(ToFloat4(*this), XMQuaternionNormalize(LoadFloat4(*this)));
    }

    void Quaternion::GetAxisAngle(Vector3D& Axis, float& Angle) const
    {
        XMVECTOR Vec;
        XMQuaternionToAxisAngle(&Vec, &Angle, LoadFloat4(*this));
        XMStoreFloat3(ToFloat3(Axis), Vec);
    }

    void Quaternion::Conjugate()
    {
        XMStoreFloat4(ToFloat4(*this), XMQuaternionConjugate(LoadFloat4(*this)));
    }

    Vector3D Quaternion::Rotate(const Vector3D& Vec) const
    {
        Vector3D OutVec;
        XMStoreFloat3(ToFloat3(OutVec), XMVector3Rotate(LoadFloat3(Vec), LoadFloat4(*this)));
        return OutVec;
    }

    Quaternion& Quaternion::operator+=(const Quaternion& Other) noexcept
    {
        XMVECTOR Vec = XMVectorAdd(LoadFloat4(*this), LoadFloat4(Other));
        XMStoreFloat4(ToFloat4(*this), Vec);
        return *this;
    }

    Quaternion& Quaternion::operator+=(value_type Val) noexcept
    {
        XMVECTOR Vec = XMVectorAdd(LoadFloat4(*this), XMVectorReplicate(Val));
        XMStoreFloat4(ToFloat4(*this), Vec);
        return *this;
    }

    Quaternion& Quaternion::operator-=(const Quaternion& Other) noexcept
    {
        XMVECTOR Vec = XMVectorSubtract(LoadFloat4(*this), LoadFloat4(Other));
        XMStoreFloat4(ToFloat4(*this), Vec);
        return *this;
    }

    Quaternion& Quaternion::operator-=(value_type Val) noexcept
    {
        XMVECTOR Vec = XMVectorSubtract(LoadFloat4(*this), XMVectorReplicate(Val));
        XMStoreFloat4(ToFloat4(*this), Vec);
        return *this;
    }

    Quaternion& Quaternion::operator*=(const Quaternion& Other) noexcept
    {
        XMVECTOR Vec = XMQuaternionMultiply(LoadFloat4(*this), LoadFloat4(Other));
        XMStoreFloat4(ToFloat4(*this), Vec);
        return *this;
    }

    Quaternion& Quaternion::operator*=(value_type Val) noexcept
    {
        XMVECTOR Vec = XMQuaternionMultiply(LoadFloat4(*this), XMVectorReplicate(Val));
        XMStoreFloat4(ToFloat4(*this), Vec);
        return *this;
    }

    Quaternion& Quaternion::operator/=(const Quaternion& Other) noexcept
    {
        XMVECTOR Vec = XMVectorDivide(LoadFloat4(*this), LoadFloat4(Other));
        XMStoreFloat4(ToFloat4(*this), Vec);
        return *this;
    }

    Quaternion& Quaternion::operator/=(value_type Val) noexcept
    {
        XMVECTOR Vec = XMVectorDivide(LoadFloat4(*this), XMVectorReplicate(Val));
        XMStoreFloat4(ToFloat4(*this), Vec);
        return *this;
    }
} // namespace Neon
