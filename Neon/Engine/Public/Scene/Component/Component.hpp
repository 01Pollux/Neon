#pragma once

namespace flecs
{
    struct world;
    struct entity;
} // namespace flecs

namespace Neon::Scene
{
    using Actor = flecs::entity;
}

#define NEON_EXPORT_COMPONENT() \
    static void Register(       \
        flecs::world& World);

#define NEON_IMPLEMENT_COMPONENT(ClassName) \
    void ClassName::Register(               \
        flecs::world& World)

#define NEON_REGISTER_COMPONENT(ClassName) \
    ClassName::Register(World);
