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

    void RuntimeScene::ApplyTo(
        MergeType Type) const
    {
        NEON_ASSERT(Type == MergeType::Merge || Type == MergeType::Replace, "Invalid merge type.");

        auto World    = EntityWorld::Get();
        auto SceneTag = EntityWorld::Get().component<Scene::Component::SceneEntity>();

        // Replace the scene tag for the world to our scene tag.
        if (Type == MergeType::Replace)
        {
            World.add(SceneTag, m_SceneTag);
        }
        else
        {
            // Check if the scene tag is already present in the world.
            if (!World.has(SceneTag, m_SceneTag))
            {
                World.add(SceneTag, m_SceneTag);
            }
            // Else clone the entities from the scene to the world.
            else
            {
                auto OtherScene = World.target(SceneTag);

                // Copy our scene entities to the world.
                std::vector<EntityHandle> Entities;
                World.filter_builder()
                    .with(SceneTag, m_SceneTag)
                    .build()
                    .each([&Entities](flecs::entity_t EntHandle)
                          { Entities.push_back(EntHandle); });

                for (auto& EntHandle : Entities)
                {
                    EntHandle.CloneToRoot(m_SceneTag);
                }
            }
        }
    }

    //

    EntityHandle RuntimeScene::CreateEntity(
        const char* Name)
    {
        auto EntHandle = EntityHandle::Create(m_SceneTag, Name);
        AddEntity(EntHandle);
        return EntHandle;
    }

    void RuntimeScene::AddEntity(
        EntityHandle EntHandle)
    {
        flecs::entity Entity = EntHandle;
        Entity.add<Scene::Component::SceneEntity>(m_SceneTag);
    }

    void RuntimeScene::CloneEntity(
        EntityHandle EntHandle)
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
        }
    }
} // namespace Neon::Scene