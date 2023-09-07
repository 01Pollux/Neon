#include <EnginePCH.hpp>
#include <Scene/RuntimeScene.hpp>
#include <Scene/EntityWorld.hpp>
#include <Scene/Component/Component.hpp>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <Log/Logger.hpp>

namespace Neon::Scene
{
    /// <summary>
    /// Sander:
    /// I looked a bit more into it, turns out it's a bit of a difficult problem to address generically without complicating the APIs or hurting performance
    // One quick workaround is to give the Root entity a name, for example: world.entity("Root")
    // The reason for that is that:
    // during world cleanup root entities are cleaned up first, which should have cleaned up Root before the query entity
    // however, because the Root entity is empty, it's not stored in any archetype, which makes it really hard to find in the cleanup code
    // By adding a name (or any other component) to it, it's no longer empty and the cleanup works as expected
    // I'll add a note to the docs for now, until I come up with a better way to address this
    /// </summary>
    [[nodiscard]] static StringU8 RandomEntityName()
    {
        auto Guid = boost::uuids::random_generator()();
        auto Name = boost::uuids::to_string(Guid);
        std::replace(Name.begin(), Name.end(), '-', '_');
        return Name;
    }

    RuntimeScene::RuntimeScene() :
        m_RootEntity(EntityWorld::Get().entity(RandomEntityName().c_str()))
    {
    }

    RuntimeScene& RuntimeScene::operator=(
        RuntimeScene&& Other) noexcept
    {
        if (this != &Other)
        {
            Release();
            m_RootEntity = std::move(Other.m_RootEntity);
        }
        return *this;
    }

    RuntimeScene::~RuntimeScene()
    {
        Release();
    }

    void RuntimeScene::ApplyToRoot(
        MergeType Type) const
    {
        NEON_ASSERT(Type == RuntimeScene::MergeType::Merge || Type == RuntimeScene::MergeType::Replace, "Invalid merge type.");

        auto World     = EntityWorld::Get();
        auto SceneRoot = EntityWorld::Get().component<Scene::Component::WorldSceneRoot>();

        // Replace the scene tag for the world to our scene tag.
        if (Type == RuntimeScene::MergeType::Replace)
        {
            World.add(SceneRoot, m_RootEntity);
        }
        else
        {
            // Check if the scene tag is already present in the world.
            auto CurRoot = World.target(SceneRoot);
            if (!CurRoot)
            {
                World.add(SceneRoot, m_RootEntity);
            }
            else
            {
                // Copy our scene entities to the world.
                std::vector<EntityHandle> Entities;
                EntityWorld::GetChildrenFilter(m_RootEntity)
                    .build()
                    .each([&Entities](flecs::entity_t EntHandle)
                          { Entities.push_back(EntHandle); });

                for (auto& EntHandle : Entities)
                {
                    EntHandle.CloneToRoot(CurRoot);
                }
            }
        }
    }

    void RuntimeScene::ApplyToScene(
        MergeType     Type,
        RuntimeScene& OtherScene) const
    {
        NEON_ASSERT(Type == RuntimeScene::MergeType::Merge || Type == RuntimeScene::MergeType::Replace, "Invalid merge type.");

        auto SceneTag = EntityWorld::Get().component<Scene::Component::SceneEntity>();

        // Replace the scene tag for the world to our scene tag.
        if (Type == RuntimeScene::MergeType::Replace)
        {
            OtherScene.Release();
            OtherScene.m_RootEntity = EntityWorld::Get().entity(RandomEntityName().c_str());
        }

        std::vector<EntityHandle> Entities;
        EntityWorld::GetChildrenFilter(m_RootEntity)
            .build()
            .each([&Entities](flecs::entity_t EntHandle)
                  { Entities.push_back(EntHandle); });

        for (auto& EntHandle : Entities)
        {
            OtherScene.CloneEntity(EntHandle);
        }
    }

    EntityHandle RuntimeScene::GetRoot() const
    {
        return m_RootEntity;
    }

    //

    void RuntimeScene::CloneEntity(
        EntityHandle EntHandle) const
    {
        EntHandle.CloneToRoot(m_RootEntity);
    }

    //

    void RuntimeScene::Release()
    {
        if (m_RootEntity)
        {
            auto World    = EntityWorld::Get();
            auto SceneTag = World.component<Scene::Component::SceneEntity>();

            flecs::entity Root = m_RootEntity;
            World.remove<Scene::Component::WorldSceneRoot>(Root);

            // Remove entities that are tagged with the scene tag.
            EntityWorld::Get().delete_with(SceneTag, Root);
            Root.destruct();

            m_RootEntity = {};
        }
    }
} // namespace Neon::Scene