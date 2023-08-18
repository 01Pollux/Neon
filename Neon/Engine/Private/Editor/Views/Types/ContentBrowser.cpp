#include <EnginePCH.hpp>
#include <Editor/Views/Types/ContentBrowser.hpp>

namespace Neon::Editor::Views
{
    void ContentBrowserListener::handleFileAction(
        efsw::WatchID      WatchId,
        const std::string& dir,
        const std::string& FileName,
        efsw::Action       Action,
        std::string        OldFileName)
    {
    }

    ContentBrowser::ContentBrowser() :
        IEditorView(StandardViews::s_ContentBrowserWidgetId),
        m_ContentID(m_FileWatcher.addWatch("Content", &m_Listener, true))
    {
        m_FileWatcher.watch();
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