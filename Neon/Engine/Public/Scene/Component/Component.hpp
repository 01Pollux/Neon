#pragma once

namespace flecs
{
    struct world;
}

#define NEON_EXPORT_COMPONENT(ClassName) \
    static void Register(                \
        flecs::world& World);

#define NEON_IMPLEMENT_COMPONENT(ClassName) \
    void ClassName::Register(               \
        flecs::world& World)

#define NEON_REGISTER_COMPONENT(ClassName) \
    ClassName::Register(World);
