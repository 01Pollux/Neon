#include <EnginePCH.hpp>
#include <Editor/Views/Types/ContentBrowser.hpp>

namespace Neon::Editor::Views
{
    ContentBrowser::ContentBrowser() :
        IEditorView(StandardViews::s_ContentBrowserWidgetId)
    {
    }

    void ContentBrowser::OnUpdate()
    {
    }

    void ContentBrowser::OnRender()
    {
        if (ImGui::Begin(GetWidgetId().c_str()))
        {
        }
        ImGui::End();
    }
} // namespace Neon::Editor::Views