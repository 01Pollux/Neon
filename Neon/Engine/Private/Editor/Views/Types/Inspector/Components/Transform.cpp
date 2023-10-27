#include <EnginePCH.hpp>
#include <Scene/Component/Transform.hpp>
#include <Scene/EntityWorld.hpp>

#include <UI/Utils.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Neon::Editor
{
    static void Inspector_Component_OnTransform(
        flecs::entity Entity,
        flecs::id_t   ComponentId)
    {
        auto  Transform = static_cast<Scene::Component::Transform*>(Entity.get_mut(ComponentId));
        auto& Position  = Transform->GetPosition();
        auto  Rotation  = glm::degrees(Transform->GetRotationEuler());
        bool  Changed   = false;

        //

        if (ImGui::Button("Rebuild"))
        {
        }

        UI::Utils::DrawComponentLabel("Position");
        if (ImGui::IsItemHovered())
        {
            if (imcxx::tooltip Tooltip{})
            {
                ImGui::Text(
                    "Global: (%.3f, %.3f, %.3f)",
                    Position.x,
                    Position.y,
                    Position.z);
            }
        }

        if (UI::Utils::DragVectorComponent(Position))
        {
            Changed = true;
        }

        UI::Utils::DrawComponentLabel("Rotation");
        if (ImGui::IsItemHovered())
        {
            if (imcxx::tooltip Tooltip{})
            {
                ImGui::Text(
                    "Global: (%.3f, %.3f, %.3f)",
                    Rotation.x,
                    Rotation.y,
                    Rotation.z);
            }
        }

        if (UI::Utils::DragVectorComponent(Rotation))
        {
            Changed = true;
            Transform->SetRotationEuler(glm::radians(Rotation));
        }

        if (Changed)
        {
            Entity.modified<Scene::Component::Transform>();
        }
    }
} // namespace Neon::Editor

void Inspector_Component_OnTransform(
    flecs::entity_t EntityId,
    flecs::id_t     ComponentId)
{
    Neon::Editor::Inspector_Component_OnTransform(Neon::Scene::EntityHandle(EntityId), ComponentId);
}
