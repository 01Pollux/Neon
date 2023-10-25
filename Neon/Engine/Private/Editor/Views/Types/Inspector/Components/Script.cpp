#include <EnginePCH.hpp>
#include <Scene/Component/Script.hpp>
#include <Scene/EntityWorld.hpp>

#include <UI/Utils.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Neon::Editor
{
    static void Inspector_Component_OnScriptInstance(
        flecs::entity Entity,
        flecs::id_t   ComponentId)
    {
        bool Changed = false;
        if (Changed)
        {
            Entity.modified<Scene::Component::ScriptInstance>();
        }
    }
} // namespace Neon::Editor

void Inspector_Component_OnScriptInstance(
    flecs::entity_t EntityId,
    flecs::id_t     ComponentId)
{
    Neon::Editor::Inspector_Component_OnScriptInstance(Neon::Scene::EntityHandle(EntityId), ComponentId);
}
