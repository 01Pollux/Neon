#include <EnginePCH.hpp>
#include <Editor/Views/Types/HierachyScene.hpp>

#include <Editor/Scene/EditorEntity.hpp>
#include <Runtime/GameLogic.hpp>

#include <UI/imcxx/all_in_one.hpp>

namespace Neon::Editor::Views
{
    /// <summary>
    /// Display a menu to create a new entity.
    /// </summary>
    static void DisplayCreateEntityMenu(
        const flecs::entity&             ParentEntHandle,
        std::move_only_function<void()>& DeferredTask)
    {
        if (imcxx::menuitem_entry{ "Empty" })
        {
            DeferredTask = [ParentEntHandle]()
            {
                if (ParentEntHandle)
                {
                    Scene::EntityHandle::Create(
                        Scene::EntityWorld::GetCurrentSceneTag(),
                        ParentEntHandle);
                }
                else
                {
                    Scene::EntityHandle::Create(
                        Scene::EntityWorld::GetCurrentSceneTag());
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

    void SceneHierachy::DispalySceneObject(
        Scene::EntityHandle              EntHandle,
        std::move_only_function<void()>& DeferredTask)
    {
        flecs::entity Entity = EntHandle;

        ImGuiTableFlags TableFlags =
            ImGuiTreeNodeFlags_OpenOnDoubleClick |
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_SpanAvailWidth |
            ImGuiTreeNodeFlags_FramePadding |
            ImGuiTreeNodeFlags_SpanFullWidth;

        // Create filter to check if entity has children.
        auto ChidlrenFilter = Scene::EntityWorld::GetChildrenFilter(Entity).build();

        // If entity has no children, mark it as leaf.
        if (!ChidlrenFilter.is_true())
        {
            TableFlags |= ImGuiTreeNodeFlags_Leaf;
        }
        else
        {
            // TableFlags |= ImGuiTreeNodeFlags_Bullet;
        }

        // If entity is disabled, Make the text gray.
        bool IsDisabled = !Entity.enabled();
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
            static char Buffer[256];

            // If we clicked F2 or double click on this entity, set it as the entity to rename.
            if (ImGui::IsItemFocused())
            {
                if (ImGui::IsKeyPressed(ImGuiKey_F2) ||
                    ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    m_EntityToRename = Entity;
                    auto Name        = Entity.name();
                    strncpy_s(Buffer, Name.c_str(), Name.size());
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

                    if (ImGui::InputText("##Name", Buffer, int(std::size(Buffer)), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
                    {
                        DeferredTask = [Entity]() mutable
                        {
                            Scene::EntityHandle(Entity).SetName(Buffer);
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
            flecs::world World = Scene::EntityWorld::Get();
            World.add<Scene::Editor::SelectedForEditor>(Entity);
        }

        if (0 && !EditingName && !Entity.has<Scene::Editor::EditorSceneDoNotRemove>()) [[likely]]
        {
            if (imcxx::popup EditEntity{ imcxx::popup::context_item{} })
            {
                if (imcxx::menuitem_entry{ "Duplicate" })
                {
                    DeferredTask = [EntHandle]() mutable
                    {
                        EntHandle.CloneToParent(EntHandle.GetSceneTag());
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

                DisplayCreateEntityMenu(Entity, DeferredTask);
            }
        }

        if (HierachyNode)
        {
            ChidlrenFilter.each(
                [this, &DeferredTask](flecs::entity Entity)
                {
                    DispalySceneObject(Entity, DeferredTask);
                });
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

        auto RootFilter = Scene::EntityWorld::GetRootFilter(
                              Scene::EntityWorld::GetCurrentSceneTag())
                              .build();

        // First we need to display the children of root entity.
        std::move_only_function<void()> DeferredTask;

        {
            RootFilter.each(
                [this, &DeferredTask](flecs::entity Entity)
                {
                    DispalySceneObject(Entity, DeferredTask);
                });
        }

        if (imcxx::popup Popup{ imcxx::popup::context_window{}, nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems })
        {
            DisplayCreateEntityMenu(flecs::entity::null(), DeferredTask);
        }

        if (DeferredTask)
        {
            DeferredTask();
        }
    }
} // namespace Neon::Editor::Views