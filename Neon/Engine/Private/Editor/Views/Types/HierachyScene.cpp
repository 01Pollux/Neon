#include <EnginePCH.hpp>
#include <Editor/Views/Types/HierachyScene.hpp>

#include <Editor/Main/EditorEngine.hpp>
#include <Editor/Scene/EditorEntity.hpp>
#include <Runtime/GameLogic.hpp>

#include <UI/imcxx/all_in_one.hpp>

namespace Neon::Editor::Views
{
    void SceneHierachy::DispalySceneObject(
        Scene::EntityHandle              SelectedEntity,
        Scene::EntityHandle              EntHandle,
        std::move_only_function<void()>& DeferredTask,
        bool                             Editable)
    {
        flecs::entity Entity = EntHandle;

        ImGuiTableFlags TableFlags =
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_SpanAvailWidth |
            ImGuiTreeNodeFlags_FramePadding |
            ImGuiTreeNodeFlags_SpanFullWidth;

        // Create filter to check if entity has children.
        auto ChidlrenFilter = Scene::EntityWorld::GetChildrenFilter(Entity).build();

        // If entity has no children, mark it as leaf.
        if (!ChidlrenFilter.is_true())
        {
            TableFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
        }

        // If entity is being selected, mark it as selected.
        if (SelectedEntity == Entity)
        {
            TableFlags |= ImGuiTreeNodeFlags_Framed;
        }

        // If entity is disabled, Make the text gray.
        bool IsDisabled = !Editable || !Entity.enabled();
        if (IsDisabled)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        }

        auto Name = Entity.name();

        ImGui::SetNextItemAllowOverlap();
        imcxx::tree_node HierachyNode(
            std::bit_cast<void*>(Entity.raw_id()),
            TableFlags,
            m_EntityToRename == Entity ? "" : Name.c_str());

        bool EditingName = false;
        {
            // If we clicked F2 or double click on this entity, set it as the entity to rename.
            if (Editable && ImGui::IsItemFocused())
            {
                if (ImGui::IsKeyPressed(ImGuiKey_F2) ||
                    ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    m_EntityToRename = Entity;
                    auto Name        = Entity.name();
                    strncpy_s(m_RenameBuffer, Name.c_str(), Name.size());
                }
                else if (ImGui::IsKeyPressed(ImGuiKey_Delete))
                {
                    DeferredTask = [Entity]() mutable
                    {
                        Entity.destruct();
                    };
                }
            }

            if (m_EntityToRename == Entity)
            {
                if (ImGui::IsKeyPressed(ImGuiKey_Escape)) [[unlikely]]
                {
                    m_EntityToRename = {};
                }
                else
                {
                    ImGui::SameLine();
                    EditingName = true;

                    // If user pressed enter, or clicked outside the text box, rename the entity.
                    if (ImGui::InputText("##Name", m_RenameBuffer, int(std::size(m_RenameBuffer)), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue) ||
                        (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && ImGui::IsMouseClicked(ImGuiMouseButton_Left)))
                    {
                        DeferredTask = [this, Entity]() mutable
                        {
                            Scene::EntityHandle(Entity).SetName(m_RenameBuffer);
                        };
                        m_EntityToRename = {};
                    }
                }
            }
        }

        if (IsDisabled)
        {
            ImGui::PopStyleColor();
        }

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_None) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            auto EditorScene = EditorEngine::Get()->GetEditorScene();
            EditorScene.add<Scene::Editor::SelectedForEditor>(Entity);
        }

        if (Editable && !EditingName) [[likely]]
        {
            if (imcxx::popup EditEntity{ imcxx::popup::context_item{} })
            {
                if (imcxx::menuitem_entry{ "Duplicate" })
                {
                    DeferredTask = [EntHandle]() mutable
                    {
                        EntHandle.CloneToParent(EntHandle.GetContainingScene());
                    };
                }

                if (imcxx::menuitem_entry{ "Delete (Recursive)" })
                {
                    DeferredTask = [EntHandle]() mutable
                    {
                        EntHandle.Delete(true);
                    };
                }

                if (imcxx::menuitem_entry{ "Delete" })
                {
                    // Delete only entity, and move children to parent.
                    DeferredTask = [EntHandle]() mutable
                    {
                        EntHandle.Delete(false);
                    };
                }

                if (IsDisabled)
                {
                    if (imcxx::menuitem_entry{ "Enable" })
                    {
                        DeferredTask = [Entity]() mutable
                        {
                            Entity.enable();
                        };
                    }
                }
                else
                {
                    if (imcxx::menuitem_entry{ "Disable" })
                    {
                        DeferredTask = [Entity]() mutable
                        {
                            Entity.disable();
                        };
                    }
                }

                ImGui::Separator();

                DisplayEntityPopup(Entity, DeferredTask);
            }
        }

        if (HierachyNode)
        {
            ChidlrenFilter.each(
                [this, SelectedEntity, &DeferredTask, Editable](flecs::entity Entity)
                {
                    DispalySceneObject(SelectedEntity, Entity, DeferredTask, Editable);
                });
        }
    }

    void SceneHierachy::DispalySceneObject(
        Scene::EntityHandle              EntHandle,
        std::move_only_function<void()>& DeferredTask,
        bool                             Editable)
    {
        auto EditorScene = EditorEngine::Get()->GetEditorScene();
        DispalySceneObject(EditorScene.target<Scene::Editor::SelectedForEditor>(), EntHandle, DeferredTask, Editable);
    }

    //

    void SceneHierachy::DisplayEditorEntities()
    {
        std::move_only_function<void()> DeferredTask;
        DispalySceneObject(EditorEngine::Get()->GetEditorCamera(), DeferredTask, false);
    }

    //

    void SceneHierachy::DisplayEntityPopup(
        const flecs::entity&             ParentEntHandle,
        std::move_only_function<void()>& DeferredTask)
    {
        if (!ParentEntHandle && imcxx::menuitem_entry{ m_DisplayEditorEntities ? "Hide Editor Entitiy" : "Display Editor Entity" })
        {
            m_DisplayEditorEntities = !m_DisplayEditorEntities;
        }

        ImGui::Separator();

        if (imcxx::menuitem_entry{ "Empty" })
        {
            DeferredTask = [ParentEntHandle]()
            {
                if (ParentEntHandle)
                {
                    Scene::EntityHandle::Create(
                        EditorEngine::Get()->GetActiveSceneTag(),
                        ParentEntHandle);
                }
                else
                {
                    Scene::EntityHandle::Create(
                        EditorEngine::Get()->GetActiveSceneTag());
                }
            };
        }

        if (imcxx::menubar_item Menu2D{ "2D" })
        {
            if (imcxx::menubar_item MenuSprite{ "Sprite" })
            {
                if (imcxx::menuitem_entry{ "Triangle" })
                {
                }

                if (imcxx::menuitem_entry{ "Quad" })
                {
                }

                if (imcxx::menuitem_entry{ "Circle" })
                {
                }

                if (imcxx::menuitem_entry{ "Polygon" })
                {
                }

                if (imcxx::menuitem_entry{ "Capsule" })
                {
                }

                if (imcxx::menuitem_entry{ "Hexagon-Flat" })
                {
                }

                if (imcxx::menuitem_entry{ "Hexagon-Pointy" })
                {
                }

                if (imcxx::menuitem_entry{ "9-Sliced" })
                {
                }

                if (imcxx::menuitem_entry{ "Isometric Diamon" })
                {
                }
            }

            if (imcxx::menubar_item MenuPhysics{ "Physics" })
            {
                if (imcxx::menuitem_entry{ "Dynamic sprite" })
                {
                }

                if (imcxx::menuitem_entry{ "Static sprite" })
                {
                }
            }

            if (imcxx::menubar_item MenuTilemap{ "Tilemap" })
            {
                if (imcxx::menuitem_entry{ "Hexagon-Flat" })
                {
                }

                if (imcxx::menuitem_entry{ "Hexagon-Pointy" })
                {
                }

                if (imcxx::menuitem_entry{ "Isometric" })
                {
                }

                if (imcxx::menuitem_entry{ "Rectangular" })
                {
                }
            }
        }

        if (imcxx::menubar_item Menu3D{ "3D" })
        {
            if (imcxx::menuitem_entry{ "Cube" })
            {
            }

            if (imcxx::menuitem_entry{ "Sphere" })
            {
            }

            if (imcxx::menuitem_entry{ "Capsule" })
            {
            }

            if (imcxx::menuitem_entry{ "Cylinder" })
            {
            }

            if (imcxx::menuitem_entry{ "Cone" })
            {
            }

            if (imcxx::menuitem_entry{ "Plane" })
            {
            }

            if (imcxx::menuitem_entry{ "Quad" })
            {
            }
        }

        if (imcxx::menubar_item MenuEffect{ "Effect" })
        {
        }

        if (imcxx::menubar_item MenuLight{ "Light" })
        {
        }

        if (imcxx::menubar_item MenuAudio{ "Audio" })
        {
        }

        if (imcxx::menubar_item MenuUI{ "UI" })
        {
        }

        if (imcxx::menubar_item MenuVolume{ "Volume" })
        {
        }

        if (imcxx::menubar_item MenuCamera{ "Camera" })
        {
            if (imcxx::menuitem_entry{ "Generic" })
            {
            }

            if (imcxx::menuitem_entry{ "Pixel perfect" })
            {
            }
        }
    }

    //

    SceneHierachy::SceneHierachy() :
        IEditorView(StandardViews::s_HierachyViewWidgetId)
    {
    }

    void SceneHierachy::OnRender()
    {
        imcxx::window Window(GetWidgetId(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
        if (!Window)
        {
            return;
        }

        if (m_EntityToRename && !m_EntityToRename.is_alive())
        {
            m_EntityToRename = {};
        }

        if (m_DisplayEditorEntities)
        {
            DisplayEditorEntities();
            ImGui::Separator();
        }

        auto RootFilter = Scene::EntityWorld::GetChildrenFilter(
                              EditorEngine::Get()->GetActiveSceneTag())
                              .build();

        std::move_only_function<void()> DeferredTask;

        // First we need to display the children of root entity.
        {
            RootFilter.each(
                [this, &DeferredTask](flecs::entity Entity)
                {
                    DispalySceneObject(Entity, DeferredTask);
                });
        }

        // Display popup for creating entities in the root
        if (imcxx::popup Popup{ imcxx::popup::context_window{}, nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems })
        {
            DisplayEntityPopup(flecs::entity::null(), DeferredTask);
        }

        if (DeferredTask)
        {
            DeferredTask();
        }
    }
} // namespace Neon::Editor::Views