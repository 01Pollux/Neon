#include <EnginePCH.hpp>
#include <Editor/Views/Types/Hierachy.hpp>

namespace Neon::Editor::Views
{
    Hierachy::Hierachy() :
        IEditorView(StandardViews::s_HierachyViewWidgetId)
    {
    }

    void Hierachy::OnUpdate()
    {
    }

    void Hierachy::OnRender()
    {
        if (ImGui::Begin(GetWidgetId().c_str()))
        {
        }
        ImGui::End();
    }
} // namespace Neon::Editor::Views