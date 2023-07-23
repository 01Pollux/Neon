#pragma once

#include <Config/Window.hpp>
#include <Config/Resource.hpp>
#include <Config/Renderer.hpp>

#include <Resource/Asset.hpp>

namespace Neon::Config
{
    struct EngineConfig
    {
        WindowConfig   Window;
        ResourceConfig Resource;
        RendererConfig Renderer;

        std::optional<Asset::Handle> LoggerAssetUid = Asset::Handle::FromString("d0b50bba-f800-4c18-a595-fd5c4b380190");
    };
} // namespace Neon::Config