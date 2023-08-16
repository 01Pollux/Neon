#include <EnginePCH.hpp>
#include <Scene/EntityWorld.hpp>
#include <Scene/Component/Component.hpp>
#include <Scene/Exports/Export.hpp>

#include <Log/Logger.hpp>

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

    flecs::entity EntityWorld::CreateEntity(
        const char* Name)
    {
        return GetWorld().entity(Name);
    }

    flecs::entity EntityWorld::CreateEntityInRoot(
        const char* Name)
    {
        auto RootEntity = GetRootEntity();
        NEON_ASSERT(RootEntity, "Root entity not found");
        return CreateEntity(Name).child_of(RootEntity);
    }

    flecs::entity EntityWorld::CreateRootEntity(
        const char* Name)
    {
        auto World  = GetWorld();
        auto Entity = World.entity(Name);
        World.add<Component::Root>(Entity);
        return Entity;
    }

    flecs::entity EntityWorld::GetRootEntity()
    {
        return GetWorld().target<Component::Root>();
    }

    void EntityWorld::SetRootEntity(
        const flecs::entity& Entity)
    {
        GetWorld().add<Component::Root>(Entity);
    }
} // namespace Neon::Scene