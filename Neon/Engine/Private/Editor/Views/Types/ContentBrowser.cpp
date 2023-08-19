#include <EnginePCH.hpp>
#include <Editor/Views/Types/ContentBrowser.hpp>
#include <Editor/Main/EditorEngine.hpp>

#include <ImGuiUtils/imcxx/all_in_one.hpp>

#include <Asset/Storage.hpp>

namespace Neon::Editor::Views
{
    void ContentBrowser::FileListener::handleFileAction(
        efsw::WatchID      WatchId,
        const std::string& dir,
        const std::string& FileName,
        efsw::Action       Action,
        std::string        OldFileName)
    {
        auto ContentPackage = Editor::EditorEngine::Get()->GetContentPackage();

        switch (Action)
        {
        case efsw::Action::Add:
        {
            // Add Asset to m_ContentPack
            // Find asset handler

            break;
        }

        case efsw::Action::Delete:
        {
            // Remove file from m_ContentPack
            break;
        }

        case efsw::Action::Modified:
        {
            // Get Asset from m_ContentPack
            // Update Asset
            break;
        }

        case efsw::Action::Moved:
        {
            // Get Asset from m_ContentPack
            // Remove Asset from m_ContentPack
            // Add Asset to m_ContentPack
            break;
        }

        default:
            break;
        }
    }

    //

    ContentBrowser::ContentBrowser() :
        IEditorView(StandardViews::s_ContentBrowserWidgetId),
        m_ContentID(m_FileWatcher.addWatch("Content", &m_Listener, true)),
        m_DirectoryIterator("Content")
    {
        m_FileWatcher.watch();
    }

    //

    void ContentBrowser::OnUpdate()
    {
    }

    //

    void ContentBrowser::OnRender()
    {
        imcxx::window Window{ GetWidgetId(), nullptr, ImGuiWindowFlags_NoScrollbar };
        if (!Window)
        {
            return;
        }

        static int ViewSize = 124;
        const int  Columns  = std::clamp(int(ImGui::GetContentRegionAvail().x / ViewSize), 1, 64);

        imcxx::slider ViewSizeSLider("View Size", ViewSize, 0, 500);

        imcxx::shared_style BrowserStyle(
            ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f),
            ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f),
            ImGuiStyleVar_CellPadding, ImVec2(10.0f, 2.0f));

        if (imcxx::table Table{ "Data", Columns })
        {
            for (auto& [FilePath, IsFile] : m_DirectoryIterator.GetAllFiles())
            {
                if (Table.next_column())
                {
                    imcxx::shared_color OverrideIcon(ImGuiCol_Button, ImVec4{});
                    imcxx::button       IconButton(FilePath->string(), ImVec2(float(ViewSize), float(ViewSize)));
                }

                imcxx::text FileText(imcxx::text::wrapped{}, FilePath->string());
            }
        }
    }
} // namespace Neon::Editor::Views