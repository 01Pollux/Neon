#include <EnginePCH.hpp>
#include <Scene/RuntimeScene.hpp>
#include <Scene/EntityWorld.hpp>
#include <Scene/Component/Component.hpp>

#include <Log/Logger.hpp>

namespace Neon::Scene
{
    RuntimeScene::RuntimeScene() :
        m_SceneTag(EntityWorld::Get().entity())
    {
    }

    RuntimeScene& RuntimeScene::operator=(
        RuntimeScene&& Other) noexcept
    {
        if (this != &Other)
        {
            Release();
            m_SceneTag = std::move(Other.m_SceneTag);
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

        auto World    = EntityWorld::Get();
        auto SceneTag = EntityWorld::Get().component<Scene::Component::WorldSceneTag>();

        // Replace the scene tag for the world to our scene tag.
        if (Type == RuntimeScene::MergeType::Replace)
        {
            World.add(SceneTag, m_SceneTag);
        }
        else
        {
            // Check if the scene tag is already present in the world.
            auto DestScene = World.target(SceneTag);
            if (!DestScene)
            {
                World.add(SceneTag, m_SceneTag);
            }
            else
            {
                // Copy our scene entities to the world.
                std::vector<EntityHandle> Entities;
                World.filter_builder()
                    .with(SceneTag, m_SceneTag)
                    .build()
                    .each([&Entities](flecs::entity_t EntHandle)
                          { Entities.push_back(EntHandle); });

                for (auto& EntHandle : Entities)
                {
                    EntHandle.CloneToRoot(DestScene);
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
            OtherScene.m_SceneTag = EntityWorld::Get().entity();
        }

        std::vector<EntityHandle> Entities;
        EntityWorld::Get()
            .filter_builder()
            .with(SceneTag, m_SceneTag)
            .build()
            .each([&Entities](flecs::entity_t EntHandle)
                  { Entities.push_back(EntHandle); });

        for (auto& EntHandle : Entities)
        {
            EntHandle.CloneToRoot(OtherScene.m_SceneTag);
        }
    }

    EntityHandle RuntimeScene::GetTag() const
    {
        return m_SceneTag;
    }

    //

    EntityHandle RuntimeScene::CreateEntity(
        const char* Name) const
    {
        auto EntHandle = EntityHandle::Create(m_SceneTag, Name);
        AddEntity(EntHandle);
        return EntHandle;
    }

    void RuntimeScene::AddEntity(
        EntityHandle EntHandle) const
    {
        flecs::entity Entity = EntHandle;
        Entity.add<Scene::Component::SceneEntity>(m_SceneTag);
    }

    void RuntimeScene::CloneEntity(
        EntityHandle EntHandle) const
    {
        EntHandle.CloneToRoot(m_SceneTag);
    }

    //

    void RuntimeScene::Release()
    {
        if (m_SceneTag)
        {
            auto          SceneTag = EntityWorld::Get().component<Scene::Component::SceneEntity>();
            flecs::entity Scene    = m_SceneTag;

            // Remove entities that are tagged with the scene tag.
            EntityWorld::Get().delete_with(SceneTag, Scene);
            Scene.destruct();

            m_SceneTag = {};
        }
    }
} // namespace Neon::Scene