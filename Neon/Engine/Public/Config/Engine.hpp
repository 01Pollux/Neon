#pragma once

#include <Config/Window.hpp>
#include <Config/Resource.hpp>
#include <Config/Renderer.hpp>

namespace Neon::Config
{
    struct EngineConfig
    {
        WindowConfig   Window;
        ResourceConfig Resource;
        RendererConfig Renderer;
    };
} // namespace Neon::Config