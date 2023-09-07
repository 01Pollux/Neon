#include <EnginePCH.hpp>
#include <Scene/Exports/Actor.hpp>

#include <Scene/Component/Transform.hpp>

#include <Scene/Component/Component.hpp>
#include <Scene/Component/Sprite.hpp>

#include <Scene/Component/Camera.hpp>

#include <Scene/Component/Script.hpp>

#include <Scene/Component/Mesh.hpp>

#include <Scene/Component/Physics.hpp>

namespace Neon::Scene::Exports
{
    void RegisterActorComponents(
        flecs::world& World)
    {
        NEON_REGISTER_FLECS(Component::Transform);

        NEON_REGISTER_FLECS(Component::Sprite);

        NEON_REGISTER_FLECS(Component::Camera);

        NEON_REGISTER_FLECS(Component::ScriptInstance);

        NEON_REGISTER_FLECS(Component::MeshInstance);

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
        NEON_REGISTER_FLECS(Component::EditorMetaData);
        NEON_REGISTER_FLECS(Component::SceneEntity);
        NEON_REGISTER_FLECS(Component::WorldSceneRoot);
        NEON_REGISTER_FLECS(Component::ActiveSceneEntity);
        NEON_REGISTER_FLECS(Component::Sprite::CustomRenderer);
    }
} // namespace Neon::Scene::Exports