#include <EnginePCH.hpp>
#include <Scene/Component/Physics.hpp>
#include <Scene/EntityWorld.hpp>

#include <UI/Utils.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Neon::Editor
{
    static void Insecptor_Component_OnCollisionShape(
        flecs::entity Entity,
        flecs::id_t   ComponentId)
    {
        bool Changed = false;
        if (Changed)
        {
            Entity.modified<Scene::Component::CollisionShape>();
        }
    }

    static void Insecptor_Component_OnCollisionObject(
        flecs::entity Entity,
        flecs::id_t   ComponentId)
    {
        bool Changed = false;
        if (Changed)
        {
            Entity.modified<Scene::Component::CollisionObject>();
        }
    }
} // namespace Neon::Editor

void Insecptor_Component_OnCollisionShape(
    flecs::entity_t EntityId,
    flecs::id_t     ComponentId)
{
    Neon::Editor::Insecptor_Component_OnCollisionShape(Neon::Scene::EntityHandle(EntityId), ComponentId);
}

void Insecptor_Component_OnCollisionObject(
    flecs::entity_t EntityId,
    flecs::id_t     ComponentId)
{
    Neon::Editor::Insecptor_Component_OnCollisionObject(Neon::Scene::EntityHandle(EntityId), ComponentId);
}
