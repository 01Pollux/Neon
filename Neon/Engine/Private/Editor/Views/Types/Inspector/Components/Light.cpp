#include <EnginePCH.hpp>
#include <Scene/Component/Light.hpp>
#include <Scene/EntityWorld.hpp>

#include <UI/Utils.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Neon::Editor
{
    static bool Inspector_Component_OnLight(
        flecs::entity Entity,
        flecs::id_t   ComponentId)
    {
        bool Changed = false;
        if (Changed)
        {
            Entity.modified<Scene::Component::Light>();
            return true;
        }
        return false;
    }

    //

    static void Inspector_Component_OnDirectionalLight(
        flecs::entity Entity,
        flecs::id_t   ComponentId)
    {
        bool Changed = Inspector_Component_OnLight(Entity, ComponentId);
        if (Changed)
        {
            Entity.modified<Scene::Component::Light>();
        }
    }

    static void Inspector_Component_OnPointLight(
        flecs::entity Entity,
        flecs::id_t   ComponentId)
    {
        bool Changed = Inspector_Component_OnLight(Entity, ComponentId);
        if (Changed)
        {
            Entity.modified<Scene::Component::Light>();
        }
    }

    static void Inspector_Component_OnSpotLight(
        flecs::entity Entity,
        flecs::id_t   ComponentId)
    {
        bool Changed = Inspector_Component_OnLight(Entity, ComponentId);
        if (Changed)
        {
            Entity.modified<Scene::Component::Light>();
        }
    }
} // namespace Neon::Editor

void Inspector_Component_OnLight(
    flecs::entity_t EntityId,
    flecs::id_t     ComponentId)
{
}

void Inspector_Component_OnDirectionalLight(
    flecs::entity_t EntityId,
    flecs::id_t     ComponentId)
{
    Neon::Editor::Inspector_Component_OnDirectionalLight(Neon::Scene::EntityHandle(EntityId), ComponentId);
}

void Inspector_Component_OnPointLight(
    flecs::entity_t EntityId,
    flecs::id_t     ComponentId)
{
    Neon::Editor::Inspector_Component_OnPointLight(Neon::Scene::EntityHandle(EntityId), ComponentId);
}

void Inspector_Component_OnSpotLight(
    flecs::entity_t EntityId,
    flecs::id_t     ComponentId)
{
    Neon::Editor::Inspector_Component_OnSpotLight(Neon::Scene::EntityHandle(EntityId), ComponentId);
}
