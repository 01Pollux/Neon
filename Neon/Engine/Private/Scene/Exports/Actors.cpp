#include <EnginePCH.hpp>
#include <Scene/Exports/Actor.hpp>

#include <Scene/Component/Transform.hpp>

#include <Scene/Component/Component.hpp>
#include <Scene/Component/Sprite.hpp>

#include <Scene/Component/Camera.hpp>

#include <Scene/Component/Physics.hpp>

namespace Neon::Scene::Exports
{
    void RegisterActorComponents(
        flecs::world& World)
    {
        NEON_REGISTER_COMPONENT(Component::Transform);

        NEON_REGISTER_COMPONENT(Component::CanvasItem);
        NEON_REGISTER_COMPONENT(Component::Sprite);

        NEON_REGISTER_COMPONENT(Component::Camera);

        NEON_REGISTER_COMPONENT(Component::CollisionEnter);
        NEON_REGISTER_COMPONENT(Component::CollisionStay);
        NEON_REGISTER_COMPONENT(Component::CollisionExit);

        NEON_REGISTER_COMPONENT(Component::TriggerEnter);
        NEON_REGISTER_COMPONENT(Component::TriggerStay);
        NEON_REGISTER_COMPONENT(Component::TriggerExit);

        NEON_REGISTER_COMPONENT(Component::CollisionShape);
        NEON_REGISTER_COMPONENT(Component::CollisionObject);
    }
} // namespace Neon::Scene::Exports