#include <CorePCH.hpp>
#include <Module/Core.hpp>
#include <flecs/flecs.h>

namespace Neon::Module
{
    Core::Core(
        flecs::world& World)
    {
        World.module<Core>();
    }
} // namespace Neon::Module
