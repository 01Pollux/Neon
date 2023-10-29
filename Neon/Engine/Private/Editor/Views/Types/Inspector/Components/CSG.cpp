#include <EnginePCH.hpp>
#include <Scene/Component/CSG.hpp>
#include <Scene/EntityWorld.hpp>

#include <UI/Utils.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Neon::Editor
{
    static void Inspector_Component_OnCSGBrush(
        flecs::entity Entity,
        flecs::id_t   ComponentId)
    {
        auto& Brush   = static_cast<Scene::Component::CSGBrush*>(Entity.get_mut(ComponentId))->Brush;
        bool  Changed = false;

        //

        {
            if (ImGui::Button("Rebuild", { -FLT_MIN, 0.f }))
            {
                Brush.Rebuild();
                Changed = true;
            }

            ImGui::Separator();
        }

        if (Changed)
        {
            Entity.modified<Scene::Component::CSGBrush>();
        }
    }
} // namespace Neon::Editor

void Inspector_Component_OnCSGBrush(
    flecs::entity_t EntityId,
    flecs::id_t     ComponentId)
{
    Neon::Editor::Inspector_Component_OnCSGBrush(Neon::Scene::EntityHandle(EntityId), ComponentId);
}
