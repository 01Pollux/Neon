#include <EnginePCH.hpp>
#include <Scene/Impots/Actor.hpp>

#include <Scene/Component/Sprite.hpp>

namespace Neon::Scene::Imports
{
    void RegisterActorComponents(
        flecs::world& World)
    {
        Component::Sprite::Register(World);
        Component::CanvasItem::Register(World);
        Component::Sprite2::Register(World);
    }
} // namespace Neon::Scene::Imports