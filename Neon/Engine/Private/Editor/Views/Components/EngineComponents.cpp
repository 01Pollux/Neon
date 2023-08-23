#include <EnginePCH.hpp>
#include <Editor/Views/Components/EngineComponents.hpp>

#include <glm/gtc/type_ptr.hpp>

namespace Neon::Editor
{
    bool CameraComponentHandler::Dispatch(
        const flecs::entity&,
        const flecs::id& ComponentId)
    {
        return false;
    }

    bool PhysicsComponentHandler::Dispatch(
        const flecs::entity&,
        const flecs::id& ComponentId)
    {
        return false;
    }

    bool SpriteComponentHandler::Dispatch(
        const flecs::entity&,
        const flecs::id& ComponentId)
    {
        return false;
    }

    bool TransformComponentHandler::Dispatch(
        const flecs::entity& Entity,
        const flecs::id&     ComponentId)
    {
        auto& Transform = Entity.get_mut<Scene::Component::Transform>()->World;

        auto& Position = Transform.GetPosition();
        auto  Rotation = glm::degrees(Transform.GetRotationEuler());

        bool Changed = false;
        ImGui::Text("Position");
        if (ImGui::DragFloat3("##Position", glm::value_ptr(Position), 0.1f))
        {
            Changed = true;
        }

        ImGui::Text("Rotation");
        if (ImGui::DragFloat3("##Rotation", glm::value_ptr(Rotation), 0.1f))
        {
            Transform.SetRotationEuler(glm::radians(Rotation));
            Changed = true;
        }

        if (Changed)
        {
            Entity.modified<Scene::Component::Transform>();
        }

        return true;
    }
} // namespace Neon::Editor