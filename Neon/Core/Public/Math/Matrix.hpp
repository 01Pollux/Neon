#pragma once

#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

namespace Neon
{
    using Matrix3x3  = glm::mat3x3;
    using Matrix3x3I = glm::tmat3x3<int32_t>;
    using Matrix3x3U = glm::tmat3x3<uint32_t>;
    using Matrix4x4  = glm::mat4x4;
    using Matrix4x4I = glm::tmat4x4<int32_t>;
    using Matrix4x4U = glm::tmat4x4<uint32_t>;

    namespace Mat
    {
        template<typename _Ty>
        concept MatrixType = std::is_same_v<_Ty, Matrix3x3> ||
                             std::is_same_v<_Ty, Matrix3x3I> ||
                             std::is_same_v<_Ty, Matrix3x3U> ||
                             std::is_same_v<_Ty, Matrix4x4> ||
                             std::is_same_v<_Ty, Matrix4x4I> ||
                             std::is_same_v<_Ty, Matrix4x4U>;

        //

        template<MatrixType _Ty>
        inline constexpr _Ty Zero{ 0.f };

        template<MatrixType _Ty>
        inline constexpr _Ty Identity{ 1.f };
    } // namespace Mat
} // namespace Neon
