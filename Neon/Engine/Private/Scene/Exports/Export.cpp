#include <EnginePCH.hpp>
#include <Scene/Exports/Export.hpp>

#include <Scene/Exports/Math.hpp>
#include <Scene/Exports/Actor.hpp>

#include <Scene/Relation/Material.hpp>

namespace Neon::Scene::Exports
{
    void RegisterComponents(
        flecs::world& World)
    {
        RegisterMathComponents(World);
        RegisterActorComponents(World);
    }

    void RegisterRelations(
        flecs::world& World)
    {
        World.component<Relation::GroupByMaterialInstance>();
        World.component<Relation::GroupByMaterialInstance::Value>();
    }
} // namespace Neon::Scene::Exports