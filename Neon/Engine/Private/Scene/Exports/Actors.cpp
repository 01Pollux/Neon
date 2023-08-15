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
        NEON_REGISTER_FLECS(Component::Transform);

        NEON_REGISTER_FLECS(Component::Sprite);

        NEON_REGISTER_FLECS(Component::Camera);

        NEON_REGISTER_FLECS(Component::CollisionEnter);
        NEON_REGISTER_FLECS(Component::CollisionStay);
        NEON_REGISTER_FLECS(Component::CollisionExit);

        NEON_REGISTER_FLECS(Component::TriggerEnter);
        NEON_REGISTER_FLECS(Component::TriggerStay);
        NEON_REGISTER_FLECS(Component::TriggerExit);

        NEON_REGISTER_FLECS(Component::CollisionShape);
        NEON_REGISTER_FLECS(Component::CollisionObject);
    }

    void RegisterActorRelations(
        flecs::world& World)
    {
        NEON_REGISTER_FLECS(Component::Root);
        NEON_REGISTER_FLECS(Component::MainCamera);
        NEON_REGISTER_FLECS(Component::Sprite::MainRenderer);
    }
} // namespace Neon::Scene::Exports