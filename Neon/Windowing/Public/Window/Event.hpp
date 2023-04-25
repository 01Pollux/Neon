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

        struct Close
        {
            int ExitCode;
        };
    } // namespace Events

    using Event = std::variant<
        Events::SizeChanged,
        Events::Close>;
} // namespace Neon::Windowing