#include <EnginePCH.hpp>
#include <Editor/Views/Types/Hierachy.hpp>

#include <Editor/Main/EditorEngine.hpp>
#include <Editor/Scene/EditorEntity.hpp>
#include <Runtime/GameLogic.hpp>

#include <ImGuiUtils/imcxx/all_in_one.hpp>

namespace Neon::Editor::Views
{
    Hierachy::Hierachy() :
        IEditorView(StandardViews::s_HierachyViewWidgetId)
    {
        flecs::world World = EditorEngine::Get()->GetLogic()->GetEntityWorld();

        auto Root = World.lookup("_EditorRoot");

        auto A = World.entity();
        auto B = World.entity("B");
        auto C = World.entity("C");
        auto D = World.entity("D");
        auto E = World.entity("E");
        auto F = World.entity("F");
        auto G = World.entity("G");

        A.child_of(Root);
        {
            B.child_of(A);
            C.child_of(A);
            {
                D.child_of(C);
            }
            E.child_of(A);
        }

        A.add<Scene::Editor::HideInEditor>();
        F.child_of(Root);
        G.child_of(Root);
    }

    void Hierachy::OnUpdate()
    {
    }

    static void DispalySceneObject(
        flecs::entity Entity)
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

        bool ShouldGrayText = Entity.has<Scene::Editor::HideInEditor>();
        if (ShouldGrayText)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        }

        imcxx::tree_node HierachyNode(
            std::bit_cast<void*>(Entity.raw_id()),
            TableFlags,
            Entity.name().size() > 0 ? Entity.name() : "Unnamed Entity");

        if (ShouldGrayText)
        {
            ImGui::PopStyleColor();
        }

        if (HierachyNode)
        {
            ChidlrenFilter.each(&DispalySceneObject);
        }
    }

    void Hierachy::OnRender()
    {
        imcxx::window Window(GetWidgetId(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
        if (!Window)
        {
            return;
        }

        flecs::world World = EditorEngine::Get()->GetLogic()->GetEntityWorld();

        // If we are in editor mode, we need to display the editor root entity.
        // Otherwise, we need to display the game root entity. TODO
        auto Root = World.lookup("_EditorRoot");
        if (Root)
        {
            Root.children(&DispalySceneObject);
        }
    }
} // namespace Neon::Editor::Views