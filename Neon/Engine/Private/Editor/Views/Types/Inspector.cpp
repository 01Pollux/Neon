#include <EnginePCH.hpp>
#include <Editor/Views/Types/Inspector.hpp>

#include <Editor/Main/EditorEngine.hpp>
#include <Editor/Scene/EditorEntity.hpp>
#include <Scene/EntityWorld.hpp>

#include <UI/imcxx/all_in_one.hpp>

#include <Scene/Component/Transform.hpp>

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

        SelectedEntity.each(
            [&SelectedEntity](flecs::id ComponentId)
            {
                auto Component = Scene::EntityWorld::Get().component(ComponentId);
                if (!Component)
                {
                    return;
                }

                auto MetaData = Component.get<Scene::Component::EditorMetaData>();
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
                }
                else
                {
                    MetaData->RenderOnInsecptorCallback(SelectedEntity, ComponentId);
                }
            });

        if (!SelectedEntity.has<Scene::Editor::EditorAsset>())
        {
            // Set button in middle of screen and takes about 75% of the width
            constexpr float ButtonWidth = 140.f;
            constexpr float PopupWidth  = ButtonWidth * 1.8f;
            constexpr float PopupHeight = ButtonWidth * 2.4f;

            float Size = ImGui::GetWindowWidth();

            ImGui::SetCursorPosX((Size - ButtonWidth) / 2.0f);
            ImGui::SetNextItemWidth(Size * 0.75f);

            if (imcxx::button{ "Add Component", { ButtonWidth, 0.f } })
            {
                ImGui::OpenPopup("##AddComponent");
            }

            auto PopupPos = ImGui::GetWindowPos() + ImGui::GetCursorPos();
            PopupPos.x += (Size - PopupWidth) / 2.0f;

            ImGui::SetNextWindowPos(PopupPos);
            ImGui::SetNextWindowSize(ImVec2{ PopupWidth, PopupHeight });
            if (imcxx::popup AddComponent{ "##AddComponent", ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove })
            {
                if (imcxx::table ComponentTable{ "", 2, ImGuiTableFlags_Sortable | ImGuiTableFlags_SizingStretchSame })
                {
                    constexpr auto ComlumnFlags = ImGuiTableColumnFlags_WidthStretch;

                    ComponentTable.setup(
                        imcxx::table::setup_no_headers{},
                        imcxx::table::setup_info{
                            "Icon", ComlumnFlags | ImGuiTableColumnFlags_NoSort, ButtonWidth * 0.15f },
                        imcxx::table::setup_info{
                            "Name", ComlumnFlags | ImGuiTableColumnFlags_DefaultSort, ButtonWidth * 0.85f });

                    ecs_term_t Term{
                        .id = flecs::Any
                    };
                    ecs_iter_t Iter = ecs_term_iter(Scene::EntityWorld::Get(), &Term);

                    while (ecs_term_next(&Iter))
                    {
                        for (size_t i = 0; i < Iter.count; ++i)
                        {
                            flecs::entity ComponentId(Scene::EntityWorld::Get(), Iter.entities[i]);
                            if (SelectedEntity.has(ComponentId))
                            {
                                continue;
                            }

                            auto MetaData = ComponentId.get<Scene::Component::EditorMetaData>();
                            if (!MetaData || !MetaData->IsSceneComponent)
                            {
                                continue;
                            }

                            ComponentTable.next_column();
                            ImGui::Text("");
                            bool IsClicked = ImGui::IsItemClicked();

                            ComponentTable.next_column();
                            ImGui::TextUnformatted(ComponentId.name().c_str());
                            IsClicked |= ImGui::IsItemClicked();

                            if (IsClicked)
                            {
                                if (MetaData->OnAddInitialize)
                                {
                                    MetaData->OnAddInitialize(SelectedEntity, ComponentId);
                                }
                                ImGui::CloseCurrentPopup();
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
} // namespace Neon::Editor::Views