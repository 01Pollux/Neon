#include <EnginePCH.hpp>
#include <Scene/EntityWorld.hpp>
#include <Scene/Component/Component.hpp>
#include <Scene/Exports/Export.hpp>
#include <Scene/Component/Camera.hpp>

namespace Neon::Scene
{
    static std::mutex s_FlecsWorldMutex;

    EntityWorld::EntityWorld()
    {
        {
            std::scoped_lock Lock(s_FlecsWorldMutex);
            m_World = ecs_init();
            m_Owned = true;
        }

        // Register built-in modules.
        flecs::world World(m_World);

        World.init_builtin_components();

#ifndef NEON_DIST
        World.set<flecs::Rest>({});
        World.import <flecs::monitor>();
#endif

        // Register components & relations.
        Exports::RegisterComponents(World);
        Exports::RegisterRelations(World);

        // Create root entity with null main camera.
        m_RootEntity = World.entity(RootEntityName);
    }

    void EntityWorld::Release()
    {
        if (!m_Owned || !m_World)
        {
            return;
        }

        if (ecs_stage_is_async(m_World))
        {
            ecs_async_stage_free(m_World);
        }
        else
        {
            ecs_fini(m_World);
        }
    }

    flecs::entity EntityWorld::GetRootEntity()
    {
        return flecs::entity(m_World, m_RootEntity);
    }
} // namespace Neon::Scene