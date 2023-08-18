#include <EnginePCH.hpp>
#include <Editor/Views/Types/Console.hpp>

namespace Neon::Editor::Views
{
    Console::Console() :
        IEditorView(StandardViews::s_ConsoleViewWidgetId)
    {
    }

    void Console::OnUpdate()
    {
    }

    void Console::OnRender()
    {
        if (ImGui::Begin(GetWidgetId().c_str()))
        {
        }
        ImGui::End();
    }
} // namespace Neon::Editor::Views