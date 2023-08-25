#include <EnginePCH.hpp>
#include <Editor/Views/Types/Game.hpp>

namespace Neon::Editor::Views
{
    GameDisplay::GameDisplay() :
        IEditorView(StandardViews::s_GameViewWidgetId)
    {
    }

    void GameDisplay::OnUpdate()
    {
    }

    void GameDisplay::OnRender()
    {
        if (ImGui::Begin(GetWidgetId().c_str()))
        {
        }
        ImGui::End();
    }
} // namespace Neon::Editor::Views