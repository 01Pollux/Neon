#pragma once

#include <Config/Window.hpp>
#include <Config/Resource.hpp>
#include <Config/Graphics.hpp>

namespace Neon::Config
{
    struct EngineConfig
    {
        WindowConfig   Window;
        ResourceConfig Resource;
        GraphicsConfig Graphics;
    };
} // namespace Neon::Config