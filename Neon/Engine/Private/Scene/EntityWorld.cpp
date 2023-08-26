#include <EnginePCH.hpp>
#include <Scene/EntityWorld.hpp>
#include <Scene/Component/Component.hpp>
#include <Scene/Exports/Export.hpp>
#include <Scene/Component/Camera.hpp>

#include <Asio/Coroutines.hpp>
#include <cppcoro/when_all.hpp>
#include <cppcoro/sync_wait.hpp>

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

    //

    void EntityWorld::DeleteEntity(
        flecs::entity Entity,
        bool          WithChildren)
    {
        if (!WithChildren)
        {
            // Using coroutines since we will be locking entity when iterating the children.
            std::vector<Asio::CoLazy<>> Coroutines;

            // Move children to root entity.
            // and rename them to avoid name conflicts.
            Entity.children(
                [&Coroutines, Parent = Entity.parent()](flecs::entity Child)
                {
                    StringU8 NewName{ Child.name() };
                    StringU8 NewNameTmp = NewName;

                    size_t Idx = 0;
                    while (Parent.lookup(NewName.c_str()))
                    {
                        NewName = StringUtils::Format("{} ({})", NewNameTmp, ++Idx);
                    }

                    Coroutines.emplace_back(
                        [](flecs::entity Child,
                           flecs::entity Parent,
                           StringU8      NewName) mutable -> Asio::CoLazy<>
                        {
                            Child.set_name(nullptr);
                            Child.child_of(Parent);
                            Child.set_name(NewName.c_str());
                            co_return;
                        }(std::move(Child), Parent, std::move(NewName)));
                });

            cppcoro::sync_wait(cppcoro::when_all(std::move(Coroutines)));
        }
        Entity.destruct();
    }

    flecs::entity EntityWorld::CloneEntity(
        flecs::entity Entity,
        const char*   Name)
    {
        auto Parent = Entity.parent();

        StringU8 OldName{ Entity.name() };
        StringU8 NewName{ Name ? Name : Entity.name() };
        StringU8 NewNameTmp = NewName;

        size_t Idx = 0;
        while (Parent.lookup(NewName.c_str()))
        {
            NewName = StringUtils::Format("{} ({})", NewNameTmp, ++Idx);
        }

        Entity.set_name(nullptr);
        auto NewEntity = Entity.clone();

        Entity.set_name(OldName.c_str());
        NewEntity.set_name(NewName.c_str());

        return NewEntity;
    }
} // namespace Neon::Scene