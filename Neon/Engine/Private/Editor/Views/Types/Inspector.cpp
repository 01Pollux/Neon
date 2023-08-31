#include <EnginePCH.hpp>
#include <Editor/Views/Types/Inspector.hpp>

#include <Editor/Main/EditorEngine.hpp>
#include <Editor/Scene/EditorEntity.hpp>
#include <Scene/EntityWorld.hpp>

#include <UI/imcxx/all_in_one.hpp>

namespace Neon::Editor::Views
{
    Inspector::Inspector() :
        IEditorView(StandardViews::s_InspectorViewWidgetId)
    {
    }

    void Inspector::OnRender()
    {
        imcxx::window Window(GetWidgetId(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
        if (!Window)
        {
            return;
        }

        flecs::world World = Scene::EntityWorld::Get();

        auto SelectedEntity = World.target<Scene::Editor::SelectedForEditor>();
        if (!SelectedEntity)
        {
            return;
        }

        bool DrawAddComponent = false;
        SelectedEntity.each(
            [&SelectedEntity, &DrawAddComponent](flecs::id ComponentId)
            {
                auto Component = Scene::EntityWorld::Get().component(ComponentId);
                if (!Component)
                {
                    return;
                }

                auto MetaData  = Component.get<Scene::Component::EditorMetaData>();
                if (!MetaData)
                {
                    return;
                }

                if (MetaData->IsSceneComponent)
                {
                    auto HeaderInfo = UI::Utils::BeginComponentHeader(Component.name().c_str());
                    if (HeaderInfo)
                    {
                        MetaData->RenderOnInsecptorCallback(SelectedEntity, ComponentId);
                        UI::Utils::EndComponentHeader();
                        ImGui::Separator();
                    }

                    DrawAddComponent = true;
                }
                else
                {
                    MetaData->RenderOnInsecptorCallback(SelectedEntity, ComponentId);
                }
            });

        if (DrawAddComponent)
        {
            // Set button in middle of screen and takes about 75% of the width
            constexpr float ButtonWidth = 140.f;
            float           Size        = ImGui::GetWindowWidth();

            ImGui::SetCursorPosX((Size - ButtonWidth) / 2.0f);
            ImGui::SetNextItemWidth(Size * 0.75f);

            if (imcxx::button{ "Add Component", { ButtonWidth, 0.f } })
            {
                ImGui::OpenPopup("##AddComponent");
            }

            if (imcxx::popup AddComponent{ "##AddComponent" })
            {
                Scene::EntityWorld::Get().each(
                    flecs::Any,
                    [&SelectedEntity](
                        flecs::entity ComponentId)
                    {
                        if (!SelectedEntity.has(ComponentId))
                        {
                            if (auto MetaData = ComponentId.get<Scene::Component::EditorMetaData>())
                            {
                                if (MetaData->IsSceneComponent)
                                {
                                    ImGui::TextUnformatted(ComponentId.name().c_str());
                                }
                            }
                        }
                    });
            }
        }
    }
} // namespace Neon::Editor::Views