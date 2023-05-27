#pragma once

#include <Core/Neon.hpp>

namespace Neon::Renderer
{
    class Renderer;
} // namespace Neon::Renderer

namespace Neon::Runtime
{
    class DefaultGameEngine;

    class GameLogic
    {
        friend class DefaultGameEngine;

    public:
        GameLogic(
            DefaultGameEngine* Engine);
        NEON_CLASS_COPYMOVE(GameLogic);
        ~GameLogic();

    private:
        /// <summary>
        /// Called when the engine is ticking.
        /// </summary>
        void Tick();

        /// <summary>
        /// Called when the engine is rendering.
        /// </summary>
        void Render();

    private:
        DefaultGameEngine*       m_Engine;
        UPtr<Renderer::Renderer> m_Renderer;
    };
} // namespace Neon::Runtime