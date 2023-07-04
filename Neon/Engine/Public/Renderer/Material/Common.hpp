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
            AnisotropicClamp
        };
    } // namespace MaterialCommon
} // namespace Neon::Renderer