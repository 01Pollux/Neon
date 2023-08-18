#include <EnginePCH.hpp>
#include <Editor/Views/Types/Game.hpp>

namespace Neon::Editor::Views
{
    Game::Game() :
        IEditorView(StandardViews::s_GameViewWidgetId)
    {
    }

    void Game::OnUpdate()
    {
    }

    void Game::OnRender()
    {
        if (ImGui::Begin(GetWidgetId().c_str()))
        {
        }
        ImGui::End();
    }
} // namespace Neon::Editor::Views