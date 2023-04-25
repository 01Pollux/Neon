#include <CorePCH.hpp>

#include <Math/Matrix.hpp>
#include <DirectXMath.h>

using namespace DirectX;

namespace Neon
{
    // In Vector.cpp
    extern XMFLOAT2* ToFloat2(Vector2D& Vec);
    extern XMFLOAT3* ToFloat3(Vector3D& Vec);
    extern XMFLOAT4* ToFloat4(Vector4D& Vec);
    extern XMFLOAT4* ToFloat4(Quaternion& Vec);

    extern XMVECTOR LoadFloat2(const Vector2D& Vec);
    extern XMVECTOR LoadFloat3(const Vector3D& Vec);
    extern XMVECTOR LoadFloat4(const Vector4D& Vec);
    extern XMVECTOR LoadFloat4(const Quaternion& Vec);

    [[nodiscard]] XMFLOAT4X4* ToFloat4x4(Matrix4x4& Mat)
    {
        return std::bit_cast<XMFLOAT4X4*>(&Mat);
    }

    [[nodiscard]] const XMFLOAT4X4* ToFloat4x4(const Matrix4x4& Mat)
    {
        return std::bit_cast<const XMFLOAT4X4*>(&Mat);
    }

    [[nodiscard]] XMMATRIX LoadFloat4x4(Matrix4x4& Mat)
    {
        return XMLoadFloat4x4(ToFloat4x4(Mat));
    }

    [[nodiscard]] XMMATRIX LoadFloat4x4(const Matrix4x4& Mat)
    {
        return XMLoadFloat4x4(ToFloat4x4(Mat));
    }

    [[nodiscard]] XMFLOAT3X3* ToFloat3x3(Matrix3x3& Mat)
    {
        return std::bit_cast<XMFLOAT3X3*>(&Mat);
    }

    [[nodiscard]] const XMFLOAT3X3* ToFloat3x3(const Matrix3x3& Mat)
    {
        return std::bit_cast<const XMFLOAT3X3*>(&Mat);
    }

    [[nodiscard]] XMMATRIX LoadFloat3x3(Matrix3x3& Mat)
    {
        return XMLoadFloat3x3(ToFloat3x3(Mat));
    }

    [[nodiscard]] XMMATRIX LoadFloat3x3(const Matrix3x3& Mat)
    {
        return XMLoadFloat3x3(ToFloat3x3(Mat));
    }

    Matrix4x4::Matrix4x4(const Matrix3x3& Mat3) :
        m_Data({ Mat3(0), Mat3(1), Mat3(2), {} })
    {
    }

    Matrix4x4 Matrix4x4::Translation(const Vector3D& Vec)
    {
        Matrix4x4 Mat;
        XMStoreFloat4x4(Neon::ToFloat4x4(Mat), XMMatrixTranslationFromVector(Neon::LoadFloat3(Vec)));
        return Mat;
    }

    Matrix4x4 Matrix4x4::Scale(const Vector3D& Vec)
    {
        Matrix4x4 Mat;
        XMStoreFloat4x4(Neon::ToFloat4x4(Mat), XMMatrixScalingFromVector(Neon::LoadFloat3(Vec)));
        return Mat;
    }

    Matrix4x4 Matrix4x4::RotationX(float Angle)
    {
        Matrix4x4 Mat;
        XMStoreFloat4x4(Neon::ToFloat4x4(Mat), XMMatrixRotationX(Angle));
        return Mat;
    }

    Matrix4x4 Matrix4x4::RotationY(float Angle)
    {
        Matrix4x4 Mat;
        XMStoreFloat4x4(Neon::ToFloat4x4(Mat), XMMatrixRotationY(Angle));
        return Mat;
    }

    Matrix4x4 Matrix4x4::RotationZ(float Angle)
    {
        Matrix4x4 Mat;
        XMStoreFloat4x4(Neon::ToFloat4x4(Mat), XMMatrixRotationZ(Angle));
        return Mat;
    }

    Matrix4x4 Matrix4x4::RotationRollPitchYaw(const Vector3D& Vec)
    {
        Matrix4x4 Mat;
        XMStoreFloat4x4(Neon::ToFloat4x4(Mat), XMMatrixRotationRollPitchYaw(Vec.x(), Vec.y(), Vec.z()));
        return Mat;
    }

    Matrix4x4 Matrix4x4::RotationQuaternion(const Quaternion& Vec)
    {
        Matrix4x4 Mat;
        XMStoreFloat4x4(Neon::ToFloat4x4(Mat), XMMatrixRotationQuaternion(Neon::LoadFloat4(Vec)));
        return Mat;
    }

    Matrix4x4 Matrix4x4::RotationNormal(const Vector3D& Axis, float Angle)
    {
        Matrix4x4 Mat;
        XMStoreFloat4x4(Neon::ToFloat4x4(Mat), XMMatrixRotationNormal(Neon::LoadFloat3(Axis), Angle));
        return Mat;
    }

    Matrix4x4 Matrix4x4::RotationAxis(const Vector3D& Axis, float Angle)
    {
        Matrix4x4 Mat;
        XMStoreFloat4x4(Neon::ToFloat4x4(Mat), XMMatrixRotationAxis(Neon::LoadFloat3(Axis), Angle));
        return Mat;
    }

    Matrix4x4 Matrix4x4::LookAt(const Vector3D& EyePosition, const Vector3D& FocusPosition, const Vector3D& UpDirection)
    {
        Matrix4x4 Mat;
        XMVECTOR  vEyePosition   = Neon::LoadFloat3(EyePosition);
        XMVECTOR  vFocusPosition = Neon::LoadFloat3(FocusPosition);
        XMVECTOR  vUpDirection   = Neon::LoadFloat3(UpDirection);
        XMStoreFloat4x4(Neon::ToFloat4x4(Mat), XMMatrixLookAtLH(vEyePosition, vFocusPosition, vUpDirection));
        return Mat;
    }

    Matrix4x4 Matrix4x4::LookTo(const Vector3D& EyePosition, const Vector3D& EyeDirection, const Vector3D& UpDirection)
    {
        Matrix4x4 Mat;
        XMVECTOR  vEyePosition  = Neon::LoadFloat3(EyePosition);
        XMVECTOR  vEyeDirection = Neon::LoadFloat3(EyeDirection);
        XMVECTOR  vUpDirection  = Neon::LoadFloat3(UpDirection);
        XMStoreFloat4x4(Neon::ToFloat4x4(Mat), XMMatrixLookToLH(vEyePosition, vEyeDirection, vUpDirection));
        return Mat;
    }

    Matrix4x4 Matrix4x4::Perspective(float ViewWidth, float ViewHeight, float NearZ, float FarZ)
    {
        Matrix4x4 Mat;
        XMStoreFloat4x4(Neon::ToFloat4x4(Mat), XMMatrixPerspectiveLH(ViewWidth, ViewHeight, NearZ, FarZ));
        return Mat;
    }

    Matrix4x4 Matrix4x4::PerspectiveFov(float FovAngleY, float AspectRatio, float NearZ, float FarZ)
    {
        Matrix4x4 Mat;
        XMStoreFloat4x4(Neon::ToFloat4x4(Mat), XMMatrixPerspectiveFovLH(FovAngleY, AspectRatio, NearZ, FarZ));
        return Mat;
    }

    Matrix4x4 Matrix4x4::Orthographic(float ViewWidth, float ViewHeight, float NearZ, float FarZ)
    {
        Matrix4x4 Mat;
        XMStoreFloat4x4(Neon::ToFloat4x4(Mat), XMMatrixOrthographicLH(ViewWidth, ViewHeight, NearZ, FarZ));
        return Mat;
    }

    Matrix4x4 Matrix4x4::SRT(const Matrix4x4& Scale, const Matrix4x4& Rotation, const Matrix4x4& Translation)
    {
        return Scale * Rotation * Translation;
    }

    Vector2D Matrix4x4::DoTransform(const Vector2D& Vec) const
    {
        Vector2D Tmp;
        XMStoreFloat2(Neon::ToFloat2(Tmp), XMVector2Transform(Neon::LoadFloat2(Vec), Neon::LoadFloat4x4(*this)));
        return Tmp;
    }

    Vector2D Matrix4x4::DoTransformCoord(const Vector2D& Vec) const
    {
        Vector2D Tmp;
        XMStoreFloat2(Neon::ToFloat2(Tmp), XMVector2TransformCoord(Neon::LoadFloat2(Vec), Neon::LoadFloat4x4(*this)));
        return Tmp;
    }

    Vector2D Matrix4x4::DoTransformNormal(const Vector2D& Vec) const
    {
        Vector2D Tmp;
        XMStoreFloat2(Neon::ToFloat2(Tmp), XMVector2TransformNormal(Neon::LoadFloat2(Vec), Neon::LoadFloat4x4(*this)));
        return Tmp;
    }

    Vector3D Matrix4x4::DoTransform(const Vector3D& Vec) const
    {
        Vector3D Tmp;
        XMStoreFloat3(Neon::ToFloat3(Tmp), XMVector3Transform(Neon::LoadFloat3(Vec), Neon::LoadFloat4x4(*this)));
        return Tmp;
    }

    Vector3D Matrix4x4::DoTransformCoord(const Vector3D& Vec) const
    {
        Vector3D Tmp;
        XMStoreFloat3(Neon::ToFloat3(Tmp), XMVector3TransformCoord(Neon::LoadFloat3(Vec), Neon::LoadFloat4x4(*this)));
        return Tmp;
    }

    Vector3D Matrix4x4::DoTransformNormal(const Vector3D& Vec) const
    {
        Vector3D Tmp;
        XMStoreFloat3(Neon::ToFloat3(Tmp), XMVector3TransformNormal(Neon::LoadFloat3(Vec), Neon::LoadFloat4x4(*this)));
        return Tmp;
    }

    Vector4D Matrix4x4::DoTransform(const Vector4D& Vec) const
    {
        Vector4D Tmp;
        XMStoreFloat4(Neon::ToFloat4(Tmp), XMVector4Transform(Neon::LoadFloat4(Vec), Neon::LoadFloat4x4(*this)));
        return Tmp;
    }

    Matrix4x4 Matrix4x4::GetInverse() const
    {
        Matrix4x4 Mat;
        XMStoreFloat4x4(Neon::ToFloat4x4(Mat), XMMatrixInverse(nullptr, Neon::LoadFloat4x4(*this)));
        return Mat;
    }

    Matrix4x4 Matrix4x4::GetTranspose() const
    {
        Matrix4x4 Mat;
        XMStoreFloat4x4(Neon::ToFloat4x4(Mat), XMMatrixTranspose(Neon::LoadFloat4x4(*this)));
        return Mat;
    }

    Vector4D Matrix4x4::GetDeterminant() const
    {
        Vector4D Det;
        XMStoreFloat4(Neon::ToFloat4(Det), XMMatrixDeterminant(Neon::LoadFloat4x4(*this)));
        return Det;
    }

    std::tuple<Vector4D, Quaternion, Vector3D> Matrix4x4::GetPositionRotationScale() const
    {
        std::tuple<Vector4D, Quaternion, Vector3D> Transform;
        XMVECTOR                                   CurPosition, CurRotation, CurScale;

        XMMatrixDecompose(&CurScale, &CurRotation, &CurPosition, Neon::LoadFloat4x4(*this));
        XMStoreFloat4(Neon::ToFloat4(std::get<0>(Transform)), CurPosition);
        XMStoreFloat4(Neon::ToFloat4(std::get<1>(Transform)), CurRotation);
        XMStoreFloat3(Neon::ToFloat3(std::get<2>(Transform)), CurScale);

        return Transform;
    }

    Vector3D Matrix4x4::GetScale() const
    {
        Vector3D RetScale;
        XMVECTOR CurScale, TmpUnused;

        XMMatrixDecompose(&CurScale, &TmpUnused, &TmpUnused, Neon::LoadFloat4x4(*this));
        XMStoreFloat3(Neon::ToFloat3(RetScale), CurScale);

        return RetScale;
    }

    void Matrix4x4::SetScale(const Vector3D& Scale)
    {
        XMVECTOR CurScale, CurRotation, CurTranslation;

        XMMatrixDecompose(&CurScale, &CurRotation, &CurTranslation, Neon::LoadFloat4x4(*this));
        CurScale = Neon::LoadFloat3(Scale);

        XMMATRIX NewMat = XMMatrixScalingFromVector(CurScale) *
                          XMMatrixRotationQuaternion(CurRotation) *
                          XMMatrixTranslationFromVector(CurTranslation);
        XMStoreFloat4x4(Neon::ToFloat4x4(*this), NewMat);
    }

    Quaternion Matrix4x4::GetRotationQuat() const
    {
        Quaternion Quat;
        XMStoreFloat4(Neon::ToFloat4(Quat), XMQuaternionRotationMatrix(Neon::LoadFloat4x4(*this)));
        return Quat;
    }

    Vector3D Matrix4x4::GetPosition() const
    {
        return { m_Data[3][0], m_Data[3][1], m_Data[3][2] };
    }

    void Matrix4x4::SetPosition(const Vector4D& Position)
    {
        XMStoreFloat4(Neon::ToFloat4(m_Data[3]), Neon::LoadFloat4(Position));
    }

    Matrix4x4 Matrix4x4::operator*(const Matrix4x4& Other) const
    {
        Matrix4x4 Mat;
        XMStoreFloat4x4(Neon::ToFloat4x4(Mat), XMMatrixMultiply(Neon::LoadFloat4x4(*this), Neon::LoadFloat4x4(Other)));
        return Mat;
    }

    Matrix4x4& Matrix4x4::operator*=(const Matrix4x4& Other)
    {
        XMStoreFloat4x4(Neon::ToFloat4x4(*this), XMMatrixMultiply(Neon::LoadFloat4x4(*this), Neon::LoadFloat4x4(Other)));
        return *this;
    }

    Matrix3x3::Matrix3x3(const Matrix4x4& Mat4) :
        m_Data({ Mat4(0), Mat4(1), Mat4(2) })
    {
    }

    Matrix3x3 Matrix3x3::Scale(const Vector3D& Vec)
    {
        Matrix3x3 Mat;
        XMStoreFloat3x3(Neon::ToFloat3x3(Mat), XMMatrixScalingFromVector(Neon::LoadFloat3(Vec)));
        return Mat;
    }

    Matrix3x3 Matrix3x3::RotationX(float Angle)
    {
        Matrix3x3 Mat;
        XMStoreFloat3x3(Neon::ToFloat3x3(Mat), XMMatrixRotationX(Angle));
        return Mat;
    }

    Matrix3x3 Matrix3x3::RotationY(float Angle)
    {
        Matrix3x3 Mat;
        XMStoreFloat3x3(Neon::ToFloat3x3(Mat), XMMatrixRotationY(Angle));
        return Mat;
    }

    Matrix3x3 Matrix3x3::RotationZ(float Angle)
    {
        Matrix3x3 Mat;
        XMStoreFloat3x3(Neon::ToFloat3x3(Mat), XMMatrixRotationZ(Angle));
        return Mat;
    }

    Matrix3x3 Matrix3x3::RotationRollPitchYaw(const Vector3D& Vec)
    {
        Matrix3x3 Mat;
        XMStoreFloat3x3(Neon::ToFloat3x3(Mat), XMMatrixRotationRollPitchYaw(Vec.x(), Vec.y(), Vec.z()));
        return Mat;
    }

    Matrix3x3 Matrix3x3::RotationQuaternion(const Quaternion& Vec)
    {
        Matrix3x3 Mat;
        XMStoreFloat3x3(Neon::ToFloat3x3(Mat), XMMatrixRotationQuaternion(Neon::LoadFloat4(Vec)));
        return Mat;
    }

    Matrix3x3 Matrix3x3::RotationNormal(const Vector3D& Axis, float Angle)
    {
        Matrix3x3 Mat;
        XMStoreFloat3x3(Neon::ToFloat3x3(Mat), XMMatrixRotationNormal(Neon::LoadFloat3(Axis), Angle));
        return Mat;
    }

    Matrix3x3 Matrix3x3::RotationAxis(const Vector3D& Axis, float Angle)
    {
        Matrix3x3 Mat;
        XMStoreFloat3x3(Neon::ToFloat3x3(Mat), XMMatrixRotationAxis(Neon::LoadFloat3(Axis), Angle));
        return Mat;
    }

    Matrix3x3 Matrix3x3::SR(const Matrix3x3& Scale, const Matrix3x3& Rotation)
    {
        return Scale * Rotation;
    }

    Matrix3x3 Matrix3x3::GetInverse() const
    {
        Matrix3x3 Mat;
        XMStoreFloat3x3(Neon::ToFloat3x3(Mat), XMMatrixInverse(nullptr, Neon::LoadFloat3x3(*this)));
        return Mat;
    }

    Matrix3x3 Matrix3x3::GetTranspose() const
    {
        Matrix3x3 Mat;
        XMStoreFloat3x3(Neon::ToFloat3x3(Mat), XMMatrixTranspose(Neon::LoadFloat3x3(*this)));
        return Mat;
    }

    Vector4D Matrix3x3::GetDeterminant() const
    {
        Vector4D Det;
        XMStoreFloat4(Neon::ToFloat4(Det), XMMatrixDeterminant(Neon::LoadFloat3x3(*this)));
        return Det;
    }

    std::tuple<Quaternion, Vector3D> Matrix3x3::GetRotationScale() const
    {
        std::tuple<Quaternion, Vector3D> Transform;
        XMVECTOR                         CurPosition, CurRotation, CurScale;

        XMMatrixDecompose(&CurScale, &CurRotation, &CurPosition, Neon::LoadFloat3x3(*this));
        XMStoreFloat4(Neon::ToFloat4(std::get<0>(Transform)), CurRotation);
        XMStoreFloat3(Neon::ToFloat3(std::get<1>(Transform)), CurScale);

        return Transform;
    }

    Vector3D Matrix3x3::GetScale() const
    {
        Vector3D RetScale;
        XMVECTOR CurScale, TmpUnused;

        XMMatrixDecompose(&CurScale, &TmpUnused, &TmpUnused, Neon::LoadFloat3x3(*this));
        XMStoreFloat3(Neon::ToFloat3(RetScale), CurScale);

        return RetScale;
    }

    void Matrix3x3::SetScale(const Vector3D& Scale)
    {
        XMVECTOR CurScale, CurRotation, CurTranslation;

        XMMatrixDecompose(&CurScale, &CurRotation, &CurTranslation, Neon::LoadFloat3x3(*this));
        CurScale = Neon::LoadFloat3(Scale);

        XMMATRIX NewMat = XMMatrixScalingFromVector(CurScale) *
                          XMMatrixRotationQuaternion(CurRotation) *
                          XMMatrixTranslationFromVector(CurTranslation);
        XMStoreFloat3x3(Neon::ToFloat3x3(*this), NewMat);
    }

    Quaternion Matrix3x3::GetRotationQuat() const
    {
        Quaternion Quat;
        XMStoreFloat4(Neon::ToFloat4(Quat), XMQuaternionRotationMatrix(Neon::LoadFloat3x3(*this)));
        return Quat;
    }

    Matrix3x3 Matrix3x3::operator*(const Matrix3x3& Other) const
    {
        Matrix3x3 Mat;
        XMStoreFloat3x3(Neon::ToFloat3x3(Mat), XMMatrixMultiply(Neon::LoadFloat3x3(*this), Neon::LoadFloat3x3(Other)));
        return Mat;
    }

    Matrix3x3& Matrix3x3::operator*=(const Matrix3x3& Other)
    {
        XMStoreFloat3x3(Neon::ToFloat3x3(*this), XMMatrixMultiply(Neon::LoadFloat3x3(*this), Neon::LoadFloat3x3(Other)));
        return *this;
    }
} // namespace Neon
