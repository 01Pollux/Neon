#include <EnginePCH.hpp>
#include <Scene/Exports/Actor.hpp>

#include <Scene/Component/Camera.hpp>

namespace Neon::Scene::Exports
{
    void RegisterActorTags(
        flecs::world& World)
    {
        World.component<Tags::MainCamera>("Neon::Tags::MainCamera");
    }
} // namespace Neon::Scene::Exports