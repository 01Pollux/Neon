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

        if (Parent)
        {
            size_t Idx = 0;
            while (Parent.lookup(NewName.c_str()))
            {
                NewName = StringUtils::Format("{} ({})", NewNameTmp, ++Idx);
            }
        }
        else
        {
            auto   World = EntityWorld::Get();
            size_t Idx   = 0;
            while (World.lookup(NewName.c_str()))
            {
                NewName = StringUtils::Format("{} ({})", NewNameTmp, ++Idx);
            }
        }

        return NewName;
    }

    //

    /// <summary>
    /// Creates a unique entity name.
    /// </summary>
    [[nodiscard]] static StringU8 CreateUniqueEntityName(
        const char* Name)
    {
        return CreateUniqueEntityName({}, Name);
    }

    //

    EntityHandle EntityHandle::Create(
        EntityHandle SceneHandle,
        const char*  Name)
    {
        return EntityWorld::Get()
            .entity()
            .add<Scene::Component::SceneEntity>(SceneHandle)
            .set_name(CreateUniqueEntityName(Name).c_str());
    }

    EntityHandle EntityHandle::Create(
        EntityHandle SceneHandle,
        EntityHandle ParentHandle,
        const char*  Name)
    {
        flecs::entity Parent = ParentHandle;
        return EntityWorld::Get()
            .entity()
            .add<Scene::Component::SceneEntity>(SceneHandle)
            .child_of(Parent)
            .set_name(CreateUniqueEntityName(Parent, Name).c_str());
    }

    //

    flecs::entity EntityHandle::Get() const noexcept
    {
        return flecs::entity(EntityWorld::Get(), m_Entity);
    }

    //

    EntityHandle EntityHandle::GetSceneTag() const
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
        EntityHandle              SceneHandle,
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
            .add<Scene::Component::SceneEntity>(SceneHandle);

        return NewEntity;
    }

    /// <summary>
    /// Clones an entity and all its children while also executing a callback for each entity.
    /// </summary>
    [[nodiscard]] static Asio::CoLazy<EntityHandle> CloneEntity(
        EntityHandle              SceneHandle,
        EntityHandle              EntHandle,
        EntityHandle              NewParentHandle,
        const flecs::string_view& Name)
    {
        auto NewEntity = CloneEntityInternal(SceneHandle, EntHandle, NewParentHandle, Name);

        std::vector<EntityHandle> Children;
        EntHandle.Get().children(
            [&Children](flecs::entity Child)
            {
                Children.emplace_back(Child);
            });

        for (flecs::entity Child : Children)
        {
            co_await CloneEntity(SceneHandle, Child, NewEntity, Child.name());
        }

        co_return NewEntity;
    }

    //

    EntityHandle EntityHandle::CloneToParent(
        EntityHandle SceneHandle,
        const char*  Name) const
    {
        return CloneTo(SceneHandle, Get().parent(), Name);
    }

    EntityHandle EntityHandle::CloneTo(
        EntityHandle    SceneHandle,
        flecs::entity_t NewParent,
        const char*     Name) const
    {
        return cppcoro::sync_wait(CloneEntity(SceneHandle, *this, NewParent, flecs::string_view{ Name }));
    }

    EntityHandle EntityHandle::CloneToRoot(
        EntityHandle SceneHandle,
        const char*  Name) const
    {
        return CloneTo(SceneHandle, flecs::entity::null(), Name);
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

    flecs::filter_builder<> EntityWorld::GetRootFilter(
        EntityHandle SceneTag)
    {
        return Get()
            .filter_builder()
            .with(flecs::ChildOf, 0)
            .with<Scene::Component::SceneEntity>(SceneTag)
            .first();
    }

    flecs::entity EntityWorld::GetCurrentSceneTag()
    {
        return Get().target<Scene::Component::WorldSceneTag>();
    }

    void EntityWorld::SetCurrentSceneTag(
        flecs::entity Tag)
    {
        Get().add<Scene::Component::WorldSceneTag>(Tag);
    }
} // namespace Neon::Scene