#include <EnginePCH.hpp>
#include <Scene/Scene.hpp>

//

#include <RenderGraph/Graphs/Standard2D.hpp>
#include <Scene/Component/Camera.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Sprite.hpp>

//

namespace Neon::Scene
{
    using namespace Component;

    flecs::entity GameScene::CreateEntity(
        EntityType  Type,
        const char* Name)
    {
        switch (Type)
        {
        case EntityType::Empty:
        {
            return m_EntityWorld->entity(Name);
        }
        case EntityType::Camera2D:
        {
            auto   Actor = m_EntityWorld->entity(Name);
            Camera CameraComponent(CameraType::Orthographic);

            RG::CreateStandard2DRenderGraph(CameraComponent, Actor);
            Actor.set(std::move(CameraComponent));
            Actor.set(Transform{});

            return Actor;
        }
        case EntityType::Camera3D:
        {
            // TODO: change to 3D render graph
            auto   Actor = m_EntityWorld->entity(Name);
            Camera CameraComponent(CameraType::Perspective);

            RG::CreateStandard2DRenderGraph(CameraComponent, Actor);
            Actor.set<Camera>(std::move(CameraComponent));
            Actor.set<Transform>({});

            return Actor;
        }
        case EntityType::Sprite:
        {
            auto Actor = m_EntityWorld->entity(Name);

            Actor.set<Transform>({});
            Actor.set<Sprite>({});
            Actor.add<Sprite::MainRenderer>();

            return Actor;
        }
        default:
        {
            return flecs::entity::null();
        }
        }
    }
} // namespace Neon::Scene