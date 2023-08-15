#pragma once

#include <flecs/flecs.h>

#define NEON_EXPORT_FLECS(Class, Name)                  \
    static void RegisterFlecs(                          \
        flecs::world& World)                            \
    {                                                   \
        _HandleComponent(World.component<Class>(Name)); \
    }                                                   \
    static void _HandleComponent(                       \
        flecs::entity Component)

#define NEON_REGISTER_FLECS(ClassName) \
    ClassName::RegisterFlecs(World);

namespace Neon::Scene::Component
{
    struct Root
    {
        NEON_EXPORT_FLECS(Root, "Root")
        {
            Component.add(flecs::Exclusive);
        }
    };
} // namespace Neon::Scene::Component
