#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Neon
{
    using Vector2  = glm::vec2;
    using Vector2I = glm::ivec2;
    using Vector2U = glm::uvec2;

    using Vector3  = glm::vec3;
    using Vector3I = glm::ivec3;
    using Vector3U = glm::uvec3;

    using Vector4  = glm::vec4;
    using Vector4I = glm::ivec4;
    using Vector4U = glm::uvec4;

    using Quaternion = glm::quat;

    //

    namespace Vec
    {
        template<typename _Ty>
        concept VectorType = std::is_same_v<_Ty, Vector2> || std::is_same_v<_Ty, Vector2I> || std::is_same_v<_Ty, Vector2U> ||
                             std::is_same_v<_Ty, Vector3> || std::is_same_v<_Ty, Vector3I> || std::is_same_v<_Ty, Vector3U> ||
                             std::is_same_v<_Ty, Vector4> || std::is_same_v<_Ty, Vector4I> || std::is_same_v<_Ty, Vector4U>;

        template<typename _Ty>
        concept QuaternionType = std::is_same_v<_Ty, Quaternion>;

        namespace Impl
        {
            template<VectorType _Ty>
            constexpr _Ty MakeVec(int X, int Y)
            {
                _Ty Vec{};
                Vec.x = static_cast<_Ty::value_type>(X);
                Vec.y = static_cast<_Ty::value_type>(Y);
                return Vec;
            }
        } // namespace Impl

        //

        template<VectorType _Ty>
        inline constexpr _Ty One{ 1 };

        template<VectorType _Ty>
        inline constexpr _Ty Zero{ 0 };

        template<VectorType _Ty>
        inline constexpr _Ty Up = Impl::MakeVec<_Ty>(0, 1);

        template<VectorType _Ty>
        inline constexpr _Ty Down = Impl::MakeVec<_Ty>(0, -1);

        template<VectorType _Ty>
        inline constexpr _Ty Left = Impl::MakeVec<_Ty>(-1, 0);

        template<VectorType _Ty>
        inline constexpr _Ty Right = Impl::MakeVec<_Ty>(1, 0);

        template<VectorType _Ty>
        inline constexpr _Ty Forward{ 0, 0, 1 };

        template<VectorType _Ty>
        inline constexpr _Ty Backward{ 0, 0, -1 };

        //

        template<QuaternionType _Ty>
        inline constexpr _Ty Identity{ 0, 0, 0, 1 };
    } // namespace Vec
} // namespace Neon
