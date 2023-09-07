#include <EnginePCH.hpp>
#include <Scene/EntityWorld.hpp>

#include <Scene/Component/Component.hpp>
#include <Scene/Exports/Export.hpp>
#include <Scene/Component/Camera.hpp>

#include <Asio/Coroutines.hpp>
#include <cppcoro/when_all.hpp>
#include <cppcoro/sync_wait.hpp>
#include <queue>

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
        flecs::world World{};
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
        StringU8 NewName{ Name ? Name : "Entity" };
        StringU8 NewNameTmp = NewName;

        size_t Idx = 0;
        while (Parent.lookup(NewName.c_str()))
        {
            NewName = StringUtils::Format("{}_{}", NewNameTmp, ++Idx);
        }

        return NewName;
    }

    //

    EntityHandle EntityHandle::Create(
        EntityHandle SceneRoot,
        const char*  Name)
    {
        return Create(SceneRoot, SceneRoot, Name);
    }

    EntityHandle EntityHandle::Create(
        EntityHandle SceneRoot,
        EntityHandle ParentHandle,
        const char*  Name)
    {
        flecs::entity Parent = ParentHandle;
        return EntityWorld::Get()
            .entity()
            .add<Scene::Component::SceneEntity>(SceneRoot)
            .child_of(Parent)
            .set_name(CreateUniqueEntityName(Parent, Name).c_str());
    }

    //

    flecs::entity EntityHandle::Get() const noexcept
    {
        return flecs::entity(EntityWorld::Get(), m_Entity);
    }

    //

    EntityHandle EntityHandle::GetContainingScene() const
    {
        return Get().target<Scene::Component::SceneEntity>();
    }

    void EntityHandle::SetName(
        const char* Name)
    {
        auto Entity = Get();
        auto Parent = Entity.parent();

        Entity.set_name(CreateUniqueEntityName(Parent, Name).c_str());
    }

    //

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

    //

    /// <summary>
    /// Clones an entity and all its children.
    /// </summary>
    [[nodiscard]] static EntityHandle CloneEntityInternal(
        EntityHandle              SceneRoot,
        EntityHandle              EntHandle,
        EntityHandle              NewParentHandle,
        const flecs::string_view& Name)
    {
        flecs::entity Parent = NewParentHandle,
                      Entity = EntHandle;

        StringU8 OldName{ Entity.name() };
        StringU8 NewName{ CreateUniqueEntityName(Parent, Name) };

        Entity.set_name(nullptr);

        auto NewEntity = Entity.clone();
        if (Parent)
        {
            NewEntity.child_of(Parent);
        }

        Entity.set_name(OldName.c_str());
        NewEntity
            .set_name(NewName.c_str())
            .add<Scene::Component::SceneEntity>(SceneRoot);

        return NewEntity;
    }

    /// <summary>
    /// Clones an entity and all its children while also executing a callback for each entity.
    /// </summary>
    [[nodiscard]] static Asio::CoLazy<EntityHandle> CloneEntity(
        EntityHandle              SceneRoot,
        EntityHandle              EntHandle,
        EntityHandle              NewParentHandle,
        const flecs::string_view& Name)
    {
        auto NewEntity = CloneEntityInternal(SceneRoot, EntHandle, NewParentHandle, Name);

        std::vector<EntityHandle> Children;
        EntHandle.Get().children(
            [&Children](flecs::entity Child)
            {
                Children.emplace_back(Child);
            });

        for (flecs::entity Child : Children)
        {
            co_await CloneEntity(SceneRoot, Child, NewEntity, Child.name());
        }

        co_return NewEntity;
    }

    //

    EntityHandle EntityHandle::CloneToParent(
        EntityHandle SceneRoot,
        const char*  Name) const
    {
        return CloneTo(SceneRoot, Get().parent(), Name);
    }

    EntityHandle EntityHandle::CloneToParent(
        EntityHandle SceneRoot) const
    {
        auto Entity = Get();
        return CloneTo(SceneRoot, Entity.parent(), Entity.name());
    }

    EntityHandle EntityHandle::CloneTo(
        EntityHandle    SceneRoot,
        flecs::entity_t NewParent,
        const char*     Name) const
    {
        return cppcoro::sync_wait(CloneEntity(SceneRoot, *this, NewParent, flecs::string_view{ Name }));
    }

    EntityHandle EntityHandle::CloneTo(
        EntityHandle    SceneRoot,
        flecs::entity_t NewParent) const
    {
        return CloneTo(SceneRoot, NewParent, Get().name());
    }

    EntityHandle EntityHandle::CloneToRoot(
        EntityHandle SceneRoot,
        const char*  Name) const
    {
        return CloneTo(SceneRoot, SceneRoot, Name);
    }

    EntityHandle EntityHandle::CloneToRoot(
        EntityHandle SceneRoot) const
    {
        return CloneTo(SceneRoot, SceneRoot, Get().name());
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
        Exports::RegisterRelations(s_WorldContext->World);
        Exports::RegisterComponents(s_WorldContext->World);
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

    flecs::filter_builder<> EntityWorld::GetChildrenFilter(
        EntityHandle Parent)
    {
        return Get()
            .filter_builder()
            .term(flecs::ChildOf, Parent)
            .term(flecs::Disabled)
            .optional();
    }

    flecs::entity EntityWorld::GetCurrentScenerRoot()
    {
        return Get().target<Scene::Component::WorldSceneRoot>();
    }

    void EntityWorld::SetCurrentSceneRoot(
        flecs::entity Tag)
    {
        Get().add<Scene::Component::WorldSceneRoot>(Tag);
    }
} // namespace Neon::Scene