#pragma once

#include <Core/String.hpp>
#include <Math/Vector.hpp>
#include <Math/Size2.hpp>

namespace Neon::Windowing
{
    struct WindowInitDesc
    {
        /// <summary>
        /// Window title
        /// </summary>
        StringU8 Title = "Neon";

        /// <summary>
        /// Window size
        /// </summary>
        Size2I Size = { 800, 600 };

        /// <summary>
        /// Apply custom title bar
        /// </summary>
        bool CustomTitleBar : 1 = true;

        /// <summary>
        /// Window will start in the middle of the screen
        /// </summary>
        bool StartInMiddle : 1 = true;

        /// <summary>
        /// Window will start in full screen mode
        /// </summary>
        bool FullScreen : 1 = false;

        /// <summary>
        /// Window will start in full screen mode
        /// </summary>
        bool Maximized : 1 = false;
    };
} // namespace Neon::Windowing