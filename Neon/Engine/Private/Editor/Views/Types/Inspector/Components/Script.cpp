#include <EnginePCH.hpp>
#include <Scene/Component/Script.hpp>
#include <Scene/EntityWorld.hpp>

#include <UI/Utils.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Neon::Editor
{
    
    static void Insecptor_Component_OnScript(
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

void Insecptor_Component_OnScript(
    flecs::entity_t Entity,
    flecs::id_t     ComponentId)
{
}
