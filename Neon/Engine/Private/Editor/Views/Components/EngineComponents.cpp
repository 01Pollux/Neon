#include <EnginePCH.hpp>
#include <Editor/Views/Components/EngineComponents.hpp>

#include <Editor/Main/EditorEngine.hpp>
#include <Editor/Scene/EditorEntity.hpp>

#include <UI/Utils.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Neon::Editor
{
    bool CameraComponentHandler::Draw(
        const flecs::entity& Entity,
        const flecs::id&     ComponentId)
    {
        auto HeaderInfo = UI::Utils::BeginComponentHeader("Camera");
        if (!HeaderInfo)
        {
            return true;
        }

        auto& Camera  = *static_cast<Scene::Component::Camera*>(Entity.get_mut(ComponentId));
        bool  Changed = false;

        //

        if (ImGui::CollapsingHeader("Viewport"))
        {
            imcxx::indent Indenting;

            int         ProjectionType = int(Camera.Type);
            const char* TypeNames[]    = { "Perspective", "Orthographic" };
            if (ImGui::Combo("Projection Type", &ProjectionType, TypeNames, int(std::size(TypeNames))))
            {
                Camera.Type = Scene::Component::CameraType(ProjectionType);
            }

            switch (Camera.Type)
            {
            case Scene::Component::CameraType::Perspective:
            {
                Changed |= imcxx::drag{ "Field of View", Camera.Viewport.FieldOfView, 1.f, .0001f, 179.999f };

                if (imcxx::input{ "Near Plane", Camera.Viewport.NearPlane })
                {
                    Changed                   = true;
                    Camera.Viewport.NearPlane = std::max(Camera.Viewport.NearPlane, .0001f);
                }

                if (imcxx::input{ "Far Plane", Camera.Viewport.FarPlane })
                {
                    Changed                  = true;
                    Camera.Viewport.FarPlane = std::max(Camera.Viewport.FarPlane, Camera.Viewport.NearPlane + .0001f);
                }

                for (auto& [Size, IsWidth, Name] : {
                         std::tuple{ &Camera.Viewport.Width, true, "Client Width" },
                         std::tuple{ &Camera.Viewport.Height, false, "Client Height" } })
                {
                    bool UseClientSize = IsWidth ? Camera.Viewport.ClientWidth : Camera.Viewport.ClientHeight;
                    if (imcxx::checkbox{ Name, UseClientSize })
                    {
                        Changed = true;
                        if (IsWidth)
                        {
                            Camera.Viewport.ClientWidth = UseClientSize;
                        }
                        else
                        {
                            Camera.Viewport.ClientHeight = UseClientSize;
                        }
                    }

                    imcxx::disabled SizeState{ !UseClientSize };
                    ImGui::SameLine();
                    if (imcxx::input{ "", Size })
                    {
                        Changed = true;
                        *Size   = std::max(*Size, 1.f);
                    }
                }

                break;
            }

            case Scene::Component::CameraType::Orthographic:
            {
                bool MaintainXFov = Camera.Viewport.MaintainXFov;
                if (imcxx::checkbox{ "Maintain X Fov", MaintainXFov })
                {
                    Changed                      = true;
                    Camera.Viewport.MaintainXFov = MaintainXFov;
                }

                if (imcxx::input{ "Near Plane", Camera.Viewport.NearPlane })
                {
                    Changed                   = true;
                    Camera.Viewport.NearPlane = std::max(Camera.Viewport.NearPlane, .0001f);
                }

                if (imcxx::input{ "Far Plane", Camera.Viewport.FarPlane })
                {
                    Changed                  = true;
                    Camera.Viewport.FarPlane = std::max(Camera.Viewport.FarPlane, Camera.Viewport.NearPlane + .0001f);
                }

                for (auto [Size, IsWidth, Name] : {
                         std::tuple{ &Camera.Viewport.Width, true, "Client Width" },
                         std::tuple{ &Camera.Viewport.Height, false, "Client Height" } })
                {
                    bool UseClientSize = IsWidth ? Camera.Viewport.ClientWidth : Camera.Viewport.ClientHeight;
                    if (imcxx::checkbox{ Name, UseClientSize })
                    {
                        Changed = true;
                        if (IsWidth)
                        {
                            Camera.Viewport.ClientWidth = UseClientSize;
                        }
                        else
                        {
                            Camera.Viewport.ClientHeight = UseClientSize;
                        }
                    }

                    imcxx::disabled SizeState{ !UseClientSize };
                    ImGui::SameLine();
                    if (imcxx::input{ "", Size })
                    {
                        Changed = true;
                        *Size   = std::max(*Size, 1.f);
                    }
                }

                break;
            }
            }
        }

        //

        if (ImGui::CollapsingHeader("World"))
        {
            imcxx::indent Indenting;

            if (imcxx::input{ "Look At", glm::value_ptr(Camera.LookAt) })
            {
                Changed = true;
            }

            if (imcxx::input{ "Cull Mask", Camera.CullMask })
            {
                Changed = true;
            }

            if (imcxx::input{ "Render Priority", Camera.RenderPriority })
            {
                Changed = true;
            }
        }

        //

        UI::Utils::EndComponentHeader();

        if (Changed)
        {
            Entity.modified<Scene::Component::Transform>();
        }

        return true;
    }

    //

    bool PhysicsComponentHandler::Draw(
        const flecs::entity& Entity,
        const flecs::id&     ComponentId)
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