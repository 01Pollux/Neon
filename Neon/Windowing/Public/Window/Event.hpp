#pragma once

#include <Core/String.hpp>
#include <Math/Size2.hpp>

#include <variant>

namespace Neon::Windowing
{
    namespace Events
    {
        struct SizeChanged
        {
            Size2I NewSize;
        };
    } // namespace Events

    using Event = std::variant<
        Events::SizeChanged>;
} // namespace Neon::Windowing