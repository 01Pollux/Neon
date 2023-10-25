#include <EnginePCH.hpp>
#include <Scene/Component/Mesh.hpp>
#include <Scene/EntityWorld.hpp>

#include <UI/Utils.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Neon::Editor
{
    static void Inspector_Component_OnMeshInstance(
        flecs::entity Entity,
        flecs::id_t   ComponentId)
    {
        bool Changed = false;
        if (Changed)
        {
            Entity.modified<Scene::Component::MeshInstance>();
        }
    }
} // namespace Neon::Editor

void Inspector_Component_OnMeshInstance(
    flecs::entity_t EntityId,
    flecs::id_t     ComponentId)
{
    Neon::Editor::Inspector_Component_OnMeshInstance(Neon::Scene::EntityHandle(EntityId), ComponentId);
}
