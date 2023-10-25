#include <EnginePCH.hpp>
#include <Scene/Component/Sprite.hpp>
#include <Scene/EntityWorld.hpp>

#include <Math/Transform.hpp>
#include <UI/Utils.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Neon::Editor
{
    static void Inspector_Component_OnSprite(
        flecs::entity Entity,
        flecs::id_t   ComponentId)
    {
        auto& Sprite  = *static_cast<Scene::Component::Sprite*>(Entity.get_mut(ComponentId));
        bool  Changed = false;

        //

        UI::Utils::DrawComponentLabel("Material", false);
        UI::Utils::PropertySpacing();

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

        UI::Utils::DrawComponentLabel("Rotation");
        if (UI::Utils::DragVectorComponent(Rotation))
        {
            Changed = true;
            TextureTransform.SetRotationEuler(glm::radians(Rotation));
        }

        UI::Utils::DrawComponentLabel("Scale");
        if (UI::Utils::DragVectorComponent(Scale))
        {
            Changed = true;
            TextureTransform.SetScale(Scale);
        }

        UI::Utils::PropertySpacing();

        //

        UI::Utils::DrawComponentLabel("Color");
        Changed |= UI::Utils::DrawColorPicker("##SpriteColor", Sprite.ModulationColor);

        UI::Utils::DrawComponentLabel("Sprite Size");
        Changed |= UI::Utils::DragVectorComponent(Sprite.SpriteSize);

        //

        if (Changed)
        {
            Entity.modified<Scene::Component::Sprite>();
        }
    }
} // namespace Neon::Editor

void Inspector_Component_OnSprite(
    flecs::entity_t EntityId,
    flecs::id_t     ComponentId)
{
    Neon::Editor::Inspector_Component_OnSprite(Neon::Scene::EntityHandle(EntityId), ComponentId);
}
