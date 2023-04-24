#pragma once

namespace flecs
{
    struct world;
}

namespace Neon::Module
{
    class Core
    {
    public:
        explicit Core(
            flecs::world& World);
    };
} // namespace Neon::Module