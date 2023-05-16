#pragma once

#include <Math/Size2.hpp>
#include <Core/String.hpp>

namespace Neon::Config
{
    struct WindowConfig
    {
        String Title = STR("Neon");
        Size2I Size  = { 1280, 720 };

        bool Windowed        : 1 = false;
        bool Fullscreen      : 1 = false;
        bool WithCloseButton : 1 = true;
        bool CanResize       : 1 = true;
        bool HasTitleBar     : 1 = true;
        bool StartInMiddle   : 1 = false;
    };
} // namespace Neon::Config