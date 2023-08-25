#include <EnginePCH.hpp>
#include <Editor/Views/Components/EngineComponents.hpp>

#include <Editor/Main/EditorEngine.hpp>
#include <Editor/Scene/EditorEntity.hpp>

#include <UI/Utils.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Neon::Editor
{
    bool CameraComponentHandler::Draw(
        const flecs::entity&,
        const flecs::id& ComponentId)
    {
        return false;
    }

    bool PhysicsComponentHandler::Draw(
        const flecs::entity&,
        const flecs::id& ComponentId)
    {
        return false;
    }

    bool SpriteComponentHandler::Draw(
        const flecs::entity&,
        const flecs::id& ComponentId)
    {
        return false;
    }

    bool TransformComponentHandler::Draw(
        const flecs::entity& Entity,
        const flecs::id&     ComponentId)
    {
        auto HeaderInfo = UI::Utils::BeginComponentHeader("Transform");
        if (!HeaderInfo)
        {
            return true;
        }

        auto& Transform = Entity.get_mut<Scene::Component::Transform>()->World;
        auto  Position  = Transform.GetPosition();
        auto  Rotation  = glm::degrees(Transform.GetRotationEuler());
        bool  Changed   = false;

        //

        UI::Utils::DrawComponentLabel("Position");
        if (ImGui::IsItemHovered())
        {
            if (imcxx::tooltip PositionTt{})
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
            Transform.SetPosition(Position);
        }

        //

        UI::Utils::DrawComponentLabel("Rotation");
        if (ImGui::IsItemHovered())
        {
            if (imcxx::tooltip RotationTt{})
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
            Transform.SetRotationEuler(glm::radians(Rotation));
        }

        //

        UI::Utils::EndComponentHeader();

        if (Changed)
        {
            Entity.modified<Scene::Component::Transform>();
        }

        return true;
    }
} // namespace Neon::Editor