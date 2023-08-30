#include <EnginePCH.hpp>
#include <Scene/EntityWorld.hpp>
#include <Scene/Component/Component.hpp>
#include <Scene/Exports/Export.hpp>
#include <Scene/Component/Camera.hpp>

#include <Asio/Coroutines.hpp>
#include <cppcoro/when_all.hpp>
#include <cppcoro/sync_wait.hpp>

#include <Log/Logger.hpp>

namespace Neon::Scene
{
    namespace Component::Impl
    {
        flecs::world GetWorld()
        {
            return EntityWorld::Get();
        }
    } // namespace Component::Impl

    struct WorldContext
    {
        flecs::entity_t RootEntity{};
        flecs::world    World{};
    };

    static WorldContext* s_WorldContext = nullptr;

    //

    /// <summary>
    /// Creates a unique entity name.
    /// </summary>
    [[nodiscard]] static StringU8 CreateUniqueEntityName(
        const flecs::entity& Parent,
        const char*          Name)
    {
        StringU8 NewName{ Name ? Name : " " };
        StringU8 NewNameTmp = NewName;

        size_t Idx = 0;
        while (Parent.lookup(NewName.c_str()))
        {
            NewName = StringUtils::Format("{} ({})", NewNameTmp, ++Idx);
        }

        return NewName;
    }

    //

    EntityHandle EntityHandle::Create(
        const char* Name)
    {
        return Create(EntityWorld::GetRootEntity(), Name);
    }

    EntityHandle EntityHandle::Create(
        EntityHandle ParentHandle,
        const char*  Name)
    {
        flecs::entity Parent = ParentHandle;
        return Parent
            .world()
            .entity(CreateUniqueEntityName(Parent, Name).c_str())
            .child_of(Parent);
    }

    flecs::entity EntityHandle::Get() const noexcept
    {
        return flecs::entity(EntityWorld::Get(), m_Entity);
    }

    void EntityHandle::Delete(
        bool WithChildren)
    {
        flecs::entity Entity = Get();
        if (!WithChildren)
        {
            // Using coroutines since we will be locking entity when iterating the children.
            std::vector<Asio::CoLazy<>> Coroutines;

            // Move children to root entity.
            // and rename them to avoid name conflicts.
            Entity.children(
                [&Coroutines, Parent = Entity.parent()](flecs::entity Child)
                {
                    StringU8 NewName{ CreateUniqueEntityName(Parent, Child.name()) };

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

    void EntityHandle::Clone(
        const char* Name)
    {
        Clone(Get().parent(), Name);
    }

    EntityHandle EntityHandle::Clone(
        flecs::entity_t NewParent,
        const char*     Name)
    {
        flecs::entity Parent = EntityHandle(NewParent),
                      Entity = Get();

        StringU8 OldName{ Entity.name() };
        StringU8 NewName{ CreateUniqueEntityName(Parent, Name) };

        Entity.set_name(nullptr);
        auto NewEntity = Entity.clone();

        Entity.set_name(OldName.c_str());
        NewEntity.set_name(NewName.c_str());

        return NewEntity;
    }

    //

    void EntityWorld::Initialize()
    {
        NEON_ASSERT(!s_WorldContext);
        s_WorldContext = new WorldContext;

#ifndef NEON_DIST
        s_WorldContext->World.set<flecs::Rest>({});
        s_WorldContext->World.import <flecs::monitor>();
#endif
        // Register components & relations.
        Exports::RegisterComponents(s_WorldContext->World);
        Exports::RegisterRelations(s_WorldContext->World);

        // Create root entity with null main camera.
        s_WorldContext->RootEntity = s_WorldContext->World.entity("_Root");
    }

    void EntityWorld::Shutdown()
    {
        NEON_ASSERT(s_WorldContext);
        delete s_WorldContext;
    }

    flecs::world EntityWorld::Get()
    {
        return s_WorldContext->World.get_world();
    }

    EntityHandle EntityWorld::GetRootEntity()
    {
        return s_WorldContext->RootEntity;
    }
} // namespace Neon::Scene