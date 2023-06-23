#include <EnginePCH.hpp>
#include <Scene/Exports/Export.hpp>

#include <Scene/Exports/Math.hpp>
#include <Scene/Exports/Actor.hpp>

namespace Neon::Scene::Exports
{
    void RegisterComponents(
        flecs::world& World)
    {
        RegisterMathComponents(World);
        RegisterActorComponents(World);
    }
} // namespace Neon::Scene::Exports