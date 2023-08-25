#include <EnginePCH.hpp>
#include <Editor/Views/Types/HierachyScene.hpp>

#include <Editor/Main/EditorEngine.hpp>
#include <Editor/Scene/EditorEntity.hpp>
#include <Runtime/GameLogic.hpp>

#include <UI/imcxx/all_in_one.hpp>

namespace Neon::Editor::Views
{
    SceneHierachy::SceneHierachy() :
        IEditorView(StandardViews::s_HierachyViewWidgetId)
    {
    }

    static void DispalySceneObject(
        flecs::entity                    Entity,
        std::move_only_function<void()>& DeferredTask)
    {
        ImGuiTableFlags TableFlags =
            ImGuiTreeNodeFlags_OpenOnDoubleClick |
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_SpanAvailWidth |
            ImGuiTreeNodeFlags_SpanFullWidth;

        // Create filter to check if entity has children.
        auto ChidlrenFilter = Entity.world().filter_builder().term(flecs::ChildOf, Entity).build();
        if (!ChidlrenFilter.is_true())
        {
            TableFlags |= ImGuiTreeNodeFlags_Leaf;
        }

        bool IsHiddenInEditor = Entity.has<Scene::Editor::HideInEditor>();
        if (IsHiddenInEditor)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        }

        imcxx::tree_node HierachyNode(
            std::bit_cast<void*>(Entity.raw_id()),
            TableFlags,
            Entity.name());

        if (IsHiddenInEditor)
        {
            ImGui::PopStyleColor();
        }

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_None) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            flecs::world World = EditorEngine::Get()->GetLogic()->GetEntityWorld();
            World.add<Scene::Editor::SelectedForEditor>(Entity);
        }

        if (imcxx::popup EditEntity{ imcxx::popup::context_item{} })
        {
            if (ImGui::MenuItem("Duplicate"))
            {
                DeferredTask = [Entity]() mutable
                {
                    auto Parent = Entity.parent();

                    StringU8 OldName{ Entity.name() };
                    StringU8 NewName{ Entity.name() };

                    Entity.set_name(nullptr);
                    auto NewEntity = Entity.clone();

                    do
                    {
                        NewName += " (Copy)";
                    } while (Parent.lookup(NewName.c_str()));

                    Entity.set_name(OldName.c_str());
                    NewEntity.set_name(NewName.c_str());
                };
            }

            if (ImGui::MenuItem("Delete (Recursive)"))
            {
                DeferredTask = [Entity]
                {
                    Entity.destruct();
                };
            }

            if (ImGui::MenuItem("Delete"))
            {
                // Delete only entity, and move children to parent.
                DeferredTask = [Entity]
                {
                    Entity.children(
                        [Parent = Entity.parent()](flecs::entity Child)
                        {
                            Child.child_of(Parent);
                        });
                    Entity.destruct();
                };
            }

            if (IsHiddenInEditor)
            {
                if (ImGui::MenuItem("Show in Editor"))
                {
                    DeferredTask = [Entity]() mutable
                    {
                        Entity.remove<Scene::Editor::HideInEditor>();
                    };
                }
            }
            else
            {
                if (ImGui::MenuItem("Hide in Editor"))
                {
                    DeferredTask = [Entity]() mutable
                    {
                        Entity.add<Scene::Editor::HideInEditor>();
                    };
                }
            }
        }

        if (HierachyNode)
        {
            ChidlrenFilter.each(
                [&DeferredTask](flecs::entity Entity)
                {
                    DispalySceneObject(Entity, DeferredTask);
                });
        }
    }

    void SceneHierachy::OnRender()
    {
        imcxx::window Window(GetWidgetId(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
        if (!Window)
        {
            return;
        }

        flecs::world World = EditorEngine::Get()->GetLogic()->GetEntityWorld();

        // If we are in editor mode, we need to display the editor root entity.
        auto Root = EditorEngine::Get()->GetEditorActiveRootEntity();

        std::move_only_function<void()> DeferredTask;
        Root.children(
            [&DeferredTask](flecs::entity Entity)
            {
                DispalySceneObject(Entity, DeferredTask);
            });
        if (DeferredTask)
        {
            DeferredTask();
        }
    }
} // namespace Neon::Editor::Views