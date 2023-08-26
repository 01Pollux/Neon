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

    //

    bool PhysicsComponentHandler::Draw(
        const flecs::entity&,
        const flecs::id& ComponentId)
    {
        return false;
    }

    //

    bool SpriteComponentHandler::Draw(
        const flecs::entity& Entity,
        const flecs::id&     ComponentId)
    {
        auto HeaderInfo = UI::Utils::BeginComponentHeader("Sprite");
        if (!HeaderInfo)
        {
            return true;
        }

        auto& Sprite  = *static_cast<Scene::Component::Sprite*>(Entity.get_mut(ComponentId));
        bool  Changed = false;

        //

        UI::Utils::DrawComponentLabel("Material", false);

        //

        ImGui::Dummy({ 0.f, 5.f });

        //

        ImGui::Text("Texture Transform");
        auto& TextureTransform = Sprite.TextureTransform;
        auto& Position         = TextureTransform.GetPosition();
        auto  Rotation         = glm::degrees(TextureTransform.GetRotationEuler());
        auto  Scale            = TextureTransform.GetScale();

        //

        UI::Utils::DrawComponentLabel("Position");
        if (UI::Utils::DragVectorComponent(Position))
        {
            Changed = true;
        }

        //

        UI::Utils::DrawComponentLabel("Rotation");
        if (UI::Utils::DragVectorComponent(Rotation))
        {
            Changed = true;
            TextureTransform.SetRotationEuler(glm::radians(Rotation));
        }

        //

        UI::Utils::DrawComponentLabel("Scale");
        if (UI::Utils::DragVectorComponent(Scale))
        {
            Changed = true;
            TextureTransform.SetScale(Scale);
        }

        //

        ImGui::Dummy({ 0.f, 5.f });

        //

        UI::Utils::DrawComponentLabel("Color");
        Changed |= UI::Utils::DrawColorPicker("##SpriteColor", Sprite.ModulationColor);

        //

        UI::Utils::DrawComponentLabel("Sprite Size");
        Changed |= UI::Utils::DragVectorComponent(Sprite.SpriteSize);

        //

        UI::Utils::EndComponentHeader();

        if (Changed)
        {
            Entity.modified<Scene::Component::Transform>();
        }

        return true;
    }

    //

    bool TransformComponentHandler::Draw(
        const flecs::entity& Entity,
        const flecs::id&     ComponentId)
    {
        auto HeaderInfo = UI::Utils::BeginComponentHeader("Transform");
        if (!HeaderInfo)
        {
            return true;
        }

        auto& Transform = static_cast<Scene::Component::Transform*>(Entity.get_mut(ComponentId))->World;
        auto& Position  = Transform.GetPosition();
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