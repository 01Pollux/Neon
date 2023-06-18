#include <EnginePCH.hpp>
#include <Scene/Impots/Import.hpp>

#include <Scene/Impots/Math.hpp>
#include <Scene/Impots/Actor.hpp>

namespace Neon::Scene::Imports
{
    void Import(
        flecs::world& World)
    {
        RegisterMathComponents(World);
        RegisterActorComponents(World);
    }
} // namespace Neon::Scene::Imports