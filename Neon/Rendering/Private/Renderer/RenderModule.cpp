#include <EnginePCH.hpp>
#include <Renderer/RenderModule.hpp>

namespace Neon::Module
{
    Renderer::Renderer(
        const flecs::world& World)
    {
        World.module<Renderer>("Renderer");
    }
} // namespace Neon::Module