#pragma once

#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

namespace Neon
{
    using Matrix3x3 = glm::mat3x3;
    using Matrix4x4 = glm::mat4x4;

    namespace Mat
    {
        template<typename _Ty>
        concept MatrixType = std::is_same_v<_Ty, Matrix3x3> || std::is_same_v<_Ty, Matrix4x4>;

        //

        template<MatrixType _Ty>
        inline constexpr _Ty Zero{ 0.f };

        template<MatrixType _Ty>
        inline constexpr _Ty Identity{ 1.f };
    } // namespace Mat
} // namespace Neon
