#pragma once

#include <Resource/Asset.hpp>
#include <Resource/Pack.hpp>
#include <Resource/Manager.hpp>

#include <Core/String.hpp>
#include <Math/Vector.hpp>
#include <Math/Colors.hpp>
#include <Math/Matrix.hpp>

#include <variant>
#include <vector>
#include <list>

namespace Neon::Renderer
{
    namespace MaterialCommon
    {
        enum class Sampler : uint8_t
        {
            PointWrap,
            PointClamp,
            LinearWrap,
            LinearClamp,
            AnisotropicWrap,
            AnisotropicClamp,

            _Last
        };
    } // namespace MaterialCommon

    enum class MaterialVarType : uint8_t
    {
        _First_Constant,

        Bool = _First_Constant,
        Bool2,
        Bool3,
        Bool4,
        Int,
        Int2,
        Int3,
        Int4,
        UInt,
        UInt2,
        UInt3,
        UInt4,
        Float,
        Float2,
        Float3,
        Float4,
        Color,
        Matrix3x3,
        Matrix4x4,

        _Last_Constant = Matrix4x4,

        Buffer,
        Resource,
        RWResource,
        Sampler
    };
} // namespace Neon::Renderer