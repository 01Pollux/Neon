#pragma once

#include <Config/Window.hpp>
#include <Config/Resource.hpp>

namespace Neon::Config
{
    struct EngineConfig
    {
        WindowConfig   Window;
        ResourceConfig Resource;
    };
} // namespace Neon::Config