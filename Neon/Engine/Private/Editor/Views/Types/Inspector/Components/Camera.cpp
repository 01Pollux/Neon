#include <EnginePCH.hpp>
#include <Scene/Component/Camera.hpp>
#include <Scene/EntityWorld.hpp>

#include <RenderGraph/Graphs/Standard.hpp>

#include <UI/Utils.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Neon::Editor
{
    static void Inspector_Component_OnCamera(
        flecs::entity Entity,
        flecs::id_t   ComponentId)
    {
        auto& Camera  = *static_cast<Scene::Component::Camera*>(Entity.get_mut(ComponentId));
        bool  Changed = false;

        if (ImGui::CollapsingHeader("Render Graph"))
        {
            float HalfMaxWidth = ImGui::GetContentRegionAvail().x / 2.f - ImGui::GetStyle().ItemSpacing.x / 2.f;

            ImGui::SetNextItemWidth(HalfMaxWidth);
            if (ImGui::Button("2D"))
            {
                RG::CreateStandard2DRenderGraph(Camera.GetRenderGraph());
                Changed = true;
            }

            ImGui::SameLine();

            ImGui::SetNextItemWidth(HalfMaxWidth);
            if (ImGui::Button("3D"))
            {
                RG::CreateStandard3DRenderGraph(Camera.GetRenderGraph());
                Changed = true;
            }
        }

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

            Changed |= imcxx::input{ "Cull Mask", Camera.CullMask };
            Changed |= imcxx::input{ "Render Priority", Camera.RenderPriority };
        }

        //

        if (Changed)
        {
            Entity.modified<Scene::Component::Camera>();
        }
    }
} // namespace Neon::Editor

void Inspector_Component_OnCamera(
    flecs::entity_t EntityId,
    flecs::id_t     ComponentId)
{
    Neon::Editor::Inspector_Component_OnCamera(Neon::Scene::EntityHandle(EntityId), ComponentId);
}
