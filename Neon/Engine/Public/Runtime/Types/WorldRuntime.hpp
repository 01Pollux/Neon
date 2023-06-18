#pragma once

#include <Runtime/Runtime.hpp>
#include <Scene/Scene.hpp>

namespace Neon::Runtime
{
    class EngineWorldRuntime : public EngineRuntime
    {
    public:
        EngineWorldRuntime(
            DefaultGameEngine* Engine);

        Scene::GameScene m_Scene;
    };
} // namespace Neon::Runtime