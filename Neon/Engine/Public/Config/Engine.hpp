#pragma once

#include <Config/Window.hpp>
#include <Config/Resource.hpp>
#include <Config/Renderer.hpp>
#include <Config/Script.hpp>

namespace Neon::Config
{
    struct EngineConfig
    {
        WindowConfig   Window;
        ResourceConfig Resource;
        RendererConfig Renderer;
        ScriptConfig   Script;
    };
} // namespace Neon::Config