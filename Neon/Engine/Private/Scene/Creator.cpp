#include <EnginePCH.hpp>
#include <Scene/Scene.hpp>

//

#include <Renderer/RG/Graphs/Standard2D.hpp>
#include <Scene/Component/Camera.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Sprite.hpp>

//

namespace Neon::Scene
{
    using namespace Component;

    Actor GameScene::CreateEntity(
        EntityType  Type,
        const char* Name)
    {
        switch (Type)
        {
        case EntityType::Empty:
        {
            return m_World->entity(Name);
        }
        case EntityType::Camera2D:
        {
            auto RenderGraph = RG::CreateStandard2DRenderGraph(*this);

            auto Actor = m_World->entity(Name);

            Camera CameraComponent(std::move(RenderGraph), CameraType::Orthographic);
            Actor.set(std::move(CameraComponent));
            Actor.set(Transform{});

            return Actor;
        }
        case EntityType::Camera3D:
        {
            // TODO: change to 3D render graph
            auto RenderGraph = RG::CreateStandard2DRenderGraph(*this);

            auto Actor = m_World->entity(Name);

            Camera CameraComponent(std::move(RenderGraph), CameraType::Perspective);
            Actor.set<Camera>(std::move(CameraComponent));
            Actor.set<Transform>({});

            return Actor;
        }
        case EntityType::Sprite:
        {
            auto Actor = m_World->entity(Name);

            Actor.set(Transform{});
            Actor.set(Sprite{});

            return Actor;
        }
        default:
        {
            return flecs::entity::null();
        }
        }
    }
} // namespace Neon::Scene