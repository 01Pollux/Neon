#include <EnginePCH.hpp>
#include <Editor/Views/Types/ContentBrowser.hpp>
#include <Editor/Main/EditorEngine.hpp>
#include <Editor/Profile/Manager.hpp>

#include <FileSystem/File.hpp>
#include <OS/Clipboard.hpp>

#include <UI/imcxx/all_in_one.hpp>
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
        m_ContentID(m_FileWatcher.addWatch(Project::Get()->GetContentDirectoryPath().string(), &m_Listener, true)),
        m_DirectoryIterator(Project::Get()->GetContentDirectoryPath().string())
    {
    }

    void ContentBrowser::OnRender()
    {
        imcxx::window Window{ GetWidgetId(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse };
        if (!Window)
        {
            return;
        }

        static int ViewSize = 64;
        if (imcxx::slider{ "View Size", ViewSize, 32, 352 })
        {
            ViewSize = (ViewSize / 32) * 32;
        }
        const int Columns = std::clamp(int(ImGui::GetContentRegionAvail().x / ViewSize), 1, 64);

        imcxx::shared_style BrowserStyle(
            ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f),
            ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f),
            ImGuiStyleVar_CellPadding, ImVec2(10.0f, 2.0f));

        auto ContentPackage = Project::Get()->GetContentPackage();

        if (imcxx::table Table{ "Data", Columns })
        {
            for (auto& File : m_DirectoryIterator.GetAllFiles())
            {
                if (Table.next_column())
                {
                    auto FileName    = File.Path->stem().string();
                    auto Extension   = File.Path->has_extension() ? StringUtils::ToLower(File.Path->extension().string()) : "";
                    auto TextureInfo = GetImageIcon(Extension, File.IsFile);

                    ImVec4 IconTint(1.f, 1.f, 1.f, 1.f);
                    if (File.IsFile)
                    {
                        if (!File.MetaData || !ContentPackage->ContainsAsset(File.MetaData->GetGuid()))
                        {
                            IconTint = ImVec4(1.f, 0.5f, 0.5f, 1.f);
                        }
                    }

                    imcxx::shared_color OverrideIcon(ImGuiCol_Button, ImVec4{});
                    ImGui::ImageButton(
                        FileName.c_str(),
                        TextureInfo.TextureID,
                        { float(ViewSize), float(ViewSize) },
                        TextureInfo.MinUV,
                        TextureInfo.MaxUV,
                        {},
                        IconTint);

                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ViewSize - ImGui::CalcTextSize(FileName.c_str()).x) / 2.0f);
                    ImGui::TextWrapped(FileName.c_str());
                }
            }
        }

        DisplayPopup();
    }

    ImTextureRectInfo ContentBrowser::GetImageIcon(
        const StringU8& Extension,
        bool            IsFile)
    {
        // Get file extension
        const char* IconName;

        if (IsFile)
        {
            switch (StringUtils::Hash(Extension))
            {
            case StringUtils::Hash(".dll"):
            case StringUtils::Hash(".so"):
            case StringUtils::Hash(".dylib"):
                IconName = "Icons.DLL";
                break;

            case StringUtils::Hash(".ini"):
                IconName = "Icons.INI";
                break;

            case StringUtils::Hash(".txt"):
            case StringUtils::Hash(".log"):
                IconName = "Icons.Text";
                break;

            default:
                IconName = "Icons.File";
                break;
            }
        }
        else
        {
            IconName = "Icons.Folder";
        }

        auto Profile = Editor::ProfileManager::Get();
        return Profile->GetTexture(IconName);
    }

    void ContentBrowser::DisplayPopup()
    {
        imcxx::shared_style PopupStyle(
            ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f),
            ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f),
            ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f),
            ImGuiStyleVar_CellPadding, ImVec2(10.0f, 2.0f));
        imcxx::popup Popup{ imcxx::popup::context_window{}, "ContentBrowserPopup", ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight };
        if (!Popup)
        {
            return;
        }

        if (imcxx::menubar_item CreateMenu{ "Create" })
        {
            if (imcxx::menuitem_entry{ "Folder" })
            {
            }

            ImGui::Separator();

            if (imcxx::menuitem_entry("C# Script"))
            {
            }

            if (imcxx::menubar_item Create2D{ "2D" })
            {
                if (imcxx::menubar_item CreateSprite{ "Sprite" })
                {
                    if (imcxx::menuitem_entry{ "Triangle" })
                    {
                    }

                    if (imcxx::menuitem_entry{ "Quad" })
                    {
                    }

                    if (imcxx::menuitem_entry{ "Circle" })
                    {
                    }

                    if (imcxx::menuitem_entry{ "Polygon" })
                    {
                    }

                    if (imcxx::menuitem_entry{ "Capsule" })
                    {
                    }

                    if (imcxx::menuitem_entry{ "Hexagon-Flat" })
                    {
                    }

                    if (imcxx::menuitem_entry{ "Hexagon-Pointy" })
                    {
                    }

                    if (imcxx::menuitem_entry{ "9-Sliced" })
                    {
                    }

                    if (imcxx::menuitem_entry{ "Isometric Diamon" })
                    {
                    }
                }
            }
        }

        ImGui::Separator();

        if (imcxx::menuitem_entry{ "Show in Explorer", "Ctrl+Shift+Enter" })
        {
            FileSystem::OpenDirectoryInExplorer(m_DirectoryIterator.CurrentRoot());
        }

        if (imcxx::menuitem_entry{ "Delete", "Del" })
        {
        }

        if (imcxx::menuitem_entry{ "Rename", "F2" })
        {
        }

        if (imcxx::menuitem_entry{ "Copy Path", "Ctrl+Shift+C" })
        {
            OS::SetClipboard(EditorEngine::Get()->GetWindowHandle(), m_DirectoryIterator.CurrentRoot().string());
        }

        ImGui::Separator();

        if (imcxx::menuitem_entry{ "Import Asset...", "Ctrl+I" })
        {
            OS::SetClipboard(EditorEngine::Get()->GetWindowHandle(), m_DirectoryIterator.CurrentRoot().string());
        }

        ImGui::Separator();

        if (imcxx::menuitem_entry{ "Copy", "Ctrl+C" })
        {
        }

        if (imcxx::menuitem_entry{ "Cut", "Ctrl+X" })
        {
        }

        if (imcxx::menuitem_entry{ "Paste", "Ctrl+V" })
        {
        }

        if (imcxx::menuitem_entry{ "Duplicate", "Ctrl+D" })
        {
        }
    }
} // namespace Neon::Editor::Views