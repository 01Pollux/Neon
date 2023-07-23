#pragma once

#include <Renderer/Material/States.hpp>

#include <Core/String.hpp>
#include <Math/Vector.hpp>
#include <Math/Colors.hpp>
#include <Math/Matrix.hpp>

#include <variant>
#include <vector>
#include <list>

namespace Neon::Renderer
{
    enum class MaterialVarType : uint8_t
    {
        Constant,

        Buffer,
        Resource,
        RWResource,
        Sampler
    };
} // namespace Neon::Renderer