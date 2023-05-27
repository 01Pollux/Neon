#pragma once

#include <Core/Neon.hpp>

namespace Neon::Renderer
{
    class Renderer
    {
    public:
        void Render();
        NEON_CLASS_COPYMOVE(Renderer);
        ~Renderer();
    };
} // namespace Neon::Renderer