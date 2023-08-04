#pragma once

#include <Config/Window.hpp>
#include <Config/Resource.hpp>
#include <Config/Renderer.hpp>
#include <Scene/Scene.hpp>

namespace Neon::Config
{
    struct EngineConfig
    {
        WindowConfig           Window;
        ResourceConfig         Resource;
        RendererConfig         Renderer;
        UPtr<Scene::GameScene> FirstScene = std::make_unique<Scene::GameScene>();
    };
} // namespace Neon::Config