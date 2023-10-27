#include <EnginePCH.hpp>
#include <Scene/Component/CSG.hpp>
#include <Scene/EntityWorld.hpp>

#include <UI/Utils.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Neon::Editor
{
    static void Inspector_Component_OnCSGBox3D(
        flecs::entity Entity,
        flecs::id_t   ComponentId)
    {
        auto Box     = static_cast<Scene::Component::CSGBox3D*>(Entity.get_mut(ComponentId));
        auto Size    = Box->GetSize();
        bool Changed = false;

        //

        {
            float MaxWidth = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x / 2.f;
            if (ImGui::Button("Rebuild"))
            {
                Box->Rebuild();
                Changed = true;
            }
        }

        UI::Utils::DrawComponentLabel("Size");
        if (ImGui::IsItemHovered())
        {
            if (imcxx::tooltip Tooltip{})
            {
                ImGui::Text(
                    "Size: (%.3f, %.3f, %.3f)",
                    Size.x,
                    Size.y,
                    Size.z);
            }
        }

        if (UI::Utils::DragVectorComponent(Size))
        {
            Box->SetSize(Size);
        }

        if (Changed)
        {
            Entity.modified<Scene::Component::CSGBox3D>();
        }
    }
} // namespace Neon::Editor

void Inspector_Component_OnCSGBox3D(
    flecs::entity_t EntityId,
    flecs::id_t     ComponentId)
{
    Neon::Editor::Inspector_Component_OnCSGBox3D(Neon::Scene::EntityHandle(EntityId), ComponentId);
}
