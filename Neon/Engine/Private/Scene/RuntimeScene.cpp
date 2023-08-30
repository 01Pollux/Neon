#include <EnginePCH.hpp>
#include <Scene/RuntimeScene.hpp>
#include <Scene/EntityWorld.hpp>

#include <Log/Logger.hpp>

namespace Neon::Scene
{
    RuntimeScene::RuntimeScene()
    {
        m_RootEntity = EntityWorld::Get().entity();
    }

    RuntimeScene& Neon::Scene::RuntimeScene::operator=(
        RuntimeScene&& Other) noexcept
    {
        if (this != &Other)
        {
            try
            {
                if (m_RootEntity)
                {
                    m_RootEntity.Get().destruct();
                }
                m_RootEntity = std::exchange(Other.m_RootEntity, {});
            }
            catch (...)
            {
                m_RootEntity = {};
            }
        }
        return *this;
    }

    RuntimeScene::~RuntimeScene()
    {
        if (m_RootEntity)
        {
            m_RootEntity.Get().destruct();
        }
    }

    EntityHandle RuntimeScene::GetRoot() const
    {
        return m_RootEntity;
    }

    void RuntimeScene::Apply(
        MergeType Type)
    {
        Apply(Type, EntityWorld::GetRootEntity());
    }

    void RuntimeScene::Apply(
        MergeType    Type,
        EntityHandle DestRoot)
    {
        NEON_ASSERT(DestRoot, "Root entity cannot be null.");
        NEON_ASSERT(DestRoot != m_RootEntity, "Cannot apply a scene to itself.");
        NEON_ASSERT(Type == MergeType::Merge || Type == MergeType::Replace, "Invalid merge type.");

        flecs::entity ThisRootEnt = m_RootEntity;
        flecs::entity DestRootEnt = DestRoot;

        // Delete all the children of the root entity.
        std::vector<EntityHandle> Children;
        if (Type == MergeType::Replace)
        {
            DestRootEnt.children(
                [&Children](flecs::entity Child)
                {
                    Children.push_back(Child);
                });
            for (auto& Child : Children)
            {
                ecs_delete(EntityWorld::Get(), Child);
            }
            Children.clear();
        }

        // Iterate through all the entities in the root and copy them to the scene.
        ThisRootEnt.children(
            [&Children](flecs::entity Child)
            {
                Children.push_back(Child);
            });

        for (auto& Child : Children)
        {
            Child.Clone(DestRoot);
        }
    }
} // namespace Neon::Scene