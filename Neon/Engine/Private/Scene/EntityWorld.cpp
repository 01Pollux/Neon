#include <EnginePCH.hpp>
#include <Scene/EntityWorld.hpp>
#include <Scene/Component/Component.hpp>
#include <Scene/Exports/Export.hpp>

#include <Log/Logger.hpp>

namespace Neon::Scene
{
#if 0
    static std::mutex s_FlecsWorldMutex;
    //
    //    EntityWorld::EntityWorld()
    //    {
    //        {
    //            std::scoped_lock Lock(s_FlecsWorldMutex);
    //            m_World         = flecs::world(ecs_init());
    //            m_World.m_owned = true;
    //        }
    //
    //        // Register built-in modules.
    // #ifndef NEON_DIST
    //        m_World.set<flecs::Rest>({});
    //        m_World.import <flecs::monitor>();
    // #endif
    //
    //        // Register components & relations.
    //        Exports::RegisterComponents(m_World);
    //        Exports::RegisterRelations(m_World);
    //    }

    flecs::entity EntityWorld::CreateEntity(
        const char* Name)
    {
        return entity(Name);
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
        auto Entity = CreateEntity(Name);
        add<Component::Root>(Entity);
        return Entity;
    }

    flecs::entity EntityWorld::GetRootEntity()
    {
        return target<Component::Root>();
    }

    void EntityWorld::SetRootEntity(
        const flecs::entity& Entity)
    {
        add<Component::Root>(Entity);
    }
#endif
} // namespace Neon::Scene