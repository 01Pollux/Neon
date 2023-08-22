#include <EnginePCH.hpp>
#include <Editor/Views/Types/Hierachy.hpp>

#include <Editor/Main/EditorEngine.hpp>
#include <Runtime/GameLogic.hpp>

#include <ImGuiUtils/imcxx/all_in_one.hpp>
#include <stack>

namespace Neon::Editor::Views
{
    Hierachy::Hierachy() :
        IEditorView(StandardViews::s_HierachyViewWidgetId)
    {
        auto World = EditorEngine::Get()->GetLogic()->GetEntityWorld();

        auto Root = World.CreateEntity("_EditorRoot");
        auto A    = World.CreateEntity("A");
        auto B    = World.CreateEntity("B");
        auto C    = World.CreateEntity("C");
        auto D    = World.CreateEntity("D");
        auto E    = World.CreateEntity("E");
        auto F    = World.CreateEntity("F");
        auto G    = World.CreateEntity("G");

        A.child_of(Root);
        B.child_of(A);
        C.child_of(A);
        D.child_of(C);
        E.child_of(A);
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
            ImGuiTreeNodeFlags_SpanAvailWidth;

        // Create filter to check if entity has children.
        auto ChidlrenFilter = Entity.world().filter_builder().term(flecs::ChildOf, Entity).build();
        if (!ChidlrenFilter.is_true())
        {
            TableFlags |= ImGuiTreeNodeFlags_Leaf;
        }

        bool IsHeaderActive = ImGui::TreeNodeEx(
            std::bit_cast<void*>(Entity.raw_id()),
            TableFlags,
            Entity.name().c_str());

        if (IsHeaderActive)
        {
            ChidlrenFilter.each(&DispalySceneObject);
            ImGui::TreePop();
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
        auto Root = World.entity("_EditorRoot");
        if (Root)
        {
            Root.children(&DispalySceneObject);
        }
    }
} // namespace Neon::Editor::Views