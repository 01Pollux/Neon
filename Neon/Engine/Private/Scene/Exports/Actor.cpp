#include <EnginePCH.hpp>
#include <Scene/Exports/Actor.hpp>

#include <Scene/Component/Transform.hpp>

#include <Scene/Component/Component.hpp>
#include <Scene/Component/Sprite.hpp>

namespace Neon::Scene::Exports
{
    void RegisterActorComponents(
        flecs::world& World)
    {
        NEON_REGISTER_COMPONENT(Component::Transform);

        NEON_REGISTER_COMPONENT(Component::CanvasItem);
        NEON_REGISTER_COMPONENT(Component::Sprite);
    }
} // namespace Neon::Scene::Exports