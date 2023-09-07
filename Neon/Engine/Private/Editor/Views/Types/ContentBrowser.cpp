#include <EnginePCH.hpp>
#include <Editor/Views/Types/ContentBrowser.hpp>
#include <Editor/Main/EditorEngine.hpp>
#include <Editor/Profile/Manager.hpp>

#include <cppcoro/sync_wait.hpp>
#include <cppcoro/when_all.hpp>

#include <FileSystem/File.hpp>
#include <OS/Clipboard.hpp>

#include <UI/imcxx/all_in_one.hpp>
#include <Asset/Storage.hpp>

namespace Neon::Editor::Views
{
    using CBI = Scene::Editor::ContentBrowserItem;

    //

    /// <summary>
    /// Add Content Browser Item to the entity
    /// </summary>
    static void AddCBIEntityImpl(
        const flecs::entity&         BrowserEntity,
        const std::filesystem::path& RelativePath,
        bool                         IsDirectory)
    {
        flecs::entity TrueParent =
            RelativePath.has_parent_path() ? BrowserEntity.lookup(RelativePath.parent_path().string().c_str()) : BrowserEntity;

        auto Entity = Editor::EditorEngine::Get()->CreateEditorEntity(
            TrueParent,
            FileSystem::ConvertToUnixPath(RelativePath).string().c_str());

        Entity.add<CBI>();
        Entity.set<CBI::ItemOpType>(CBI::ItemOpType::None);

        if (IsDirectory)
        {
            Entity.set<CBI::ItemType>({ CBI::ItemType::Directory });
        }
        else
        {
            Entity.set<CBI::ItemType>({ CBI::ItemType::File });
        }
    }

    /// <summary>
    /// Add Content Browser Item to the entity
    /// </summary>
    static void AddCBIEntity(
        const flecs::entity&         BrowserEntity,
        const std::filesystem::path& AbsolutePath,
        bool                         IsDirectory)
    {
        auto RelPath = std::filesystem::relative(AbsolutePath, Project::Get()->GetContentDirectoryPath()).string();

        AddCBIEntityImpl(BrowserEntity, RelPath, IsDirectory);

        if (IsDirectory)
        {
            for (auto& Iter : std::filesystem::recursive_directory_iterator(AbsolutePath))
            {
                if (!Iter.is_regular_file() && !Iter.is_directory())
                {
                    return;
                }

                auto RelPath = std::filesystem::relative(Iter, Project::Get()->GetContentDirectoryPath()).string();
                AddCBIEntityImpl(BrowserEntity, RelPath, Iter.is_directory());
            }
        }
    }

    //

    void ContentBrowser::FileListener::handleFileAction(
        efsw::WatchID      WatchId,
        const std::string& dir,
        const std::string& FileName,
        efsw::Action       Action,
        std::string        OldFileName)
    {
        auto Path    = std::filesystem::path(dir) / FileName;
        auto RelPath = std::filesystem::relative(Path, Project::Get()->GetContentDirectoryPath()).string();

        bool IsDirectory = std::filesystem::is_directory(Path);

        Asio::CoLazy<> Coroutine;
        switch (Action)
        {
        case efsw::Action::Add:
        {
            Coroutine = [](ContentBrowser*       Browser,
                           std::filesystem::path Path,
                           bool                  IsDirectory) -> Asio::CoLazy<>
            {
                Browser->m_DirectoryIterator.DeferRefresh(Path.parent_path());
                AddCBIEntity(Browser->m_ContentBrowserEntity, Path, IsDirectory);
                co_return;
            }(m_Browser, std::move(Path), IsDirectory);
            break;
        }

        case efsw::Action::Delete:
        {
            Coroutine = [](ContentBrowser* Browser,
                           StringU8        Path) -> Asio::CoLazy<>
            {
                Browser->m_DirectoryIterator.DeferRefresh(Path);
                flecs::entity       BrowserEntity = Browser->m_ContentBrowserEntity;
                Scene::EntityHandle Entity        = BrowserEntity.lookup(Path.c_str());
                if (Entity)
                {
                    Entity.Delete(true);
                }
                co_return;
            }(m_Browser, std::move(RelPath));
            break;
        }

        case efsw::Action::Modified:
        {
            return;
        }

        case efsw::Action::Moved:
        {
            return;
        }

        default:
            break;
        }

        m_Browser->m_DeferredFileActions.emplace_back(std::move(Coroutine));
    }

    //

    ContentBrowser::ContentBrowser() :
        IEditorView(StandardViews::s_ContentBrowserWidgetId),
        m_Listener(this),
        m_ContentID(m_FileWatcher.addWatch(Project::Get()->GetContentDirectoryPath().string(), &m_Listener, true)),
        m_DirectoryIterator(Project::Get()->GetContentDirectoryPath().string()),
        m_ContentBrowserEntity(Editor::EditorEngine::Get()->CreateEditorEntity("ContentBrowser")),
        m_RootToView(m_ContentBrowserEntity)
    {
        NEON_REGISTER_FLECS(CBI);
        NEON_REGISTER_FLECS_ENUM(CBI::ItemType);
        NEON_REGISTER_FLECS_ENUM(CBI::ItemOpType);

        //

        for (auto& Iter : std::filesystem::directory_iterator(Project::Get()->GetContentDirectoryPath()))
        {
            if (!Iter.is_regular_file() && !Iter.is_directory())
            {
                return;
            }

            AddCBIEntity(m_ContentBrowserEntity, Iter, Iter.is_directory());
        }

        //

        m_FileWatcher.watch();
    }

    ContentBrowser::~ContentBrowser()
    {
        m_FileWatcher.removeWatch(m_ContentID);
        m_ContentBrowserEntity.Delete(true);
    }

    //

    void ContentBrowser::OnRender()
    {
        imcxx::window Window{ GetWidgetId(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse };
        if (!Window)
        {
            return;
        }

        imcxx::shared_style BrowserStyle(
            ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f),
            ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f),
            ImGuiStyleVar_CellPadding, ImVec2(10.0f, 2.0f));

        try
        {
            cppcoro::sync_wait(cppcoro::when_all(std::move(m_DeferredFileActions)));
            m_DirectoryIterator.Update();
        }
        catch (...)
        {
        }

        {
            DrawRootDirectories();
            ImGui::SameLine();
            DrawSearchBar();
        }

        DrawItems();
        DisplayPopup();
    }

    //

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

    //

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

    //

    void ContentBrowser::DrawRootDirectories()
    {
        auto& Directories = m_DirectoryIterator.GetRoots();
        if (imcxx::table DirectoryTree{ "Tree", int(Directories.size() * 2) + 1, ImGuiTableFlags_SizingFixedFit })
        {
            size_t JumpIndex = Directories.size();

            auto DrawName = [&JumpIndex, this](auto& Name) -> bool
            {
                if (ImGui::Selectable(Name.stem().string().c_str()))
                {
                    m_DirectoryIterator.GoBack(JumpIndex);
                    return true;
                }
                return false;
            };

            auto& Root = m_DirectoryIterator.GetMainRoot();
            if (DirectoryTree.next_column())
            {
                if (DrawName(Root))
                {
                    return;
                }
            }

            for (auto& Directory : Directories)
            {
                JumpIndex--;
                if (DirectoryTree.next_column())
                {
                    ImGui::Text(">");
                }
                if (DirectoryTree.next_column())
                {
                    if (DrawName(Directory))
                    {
                        return;
                    }
                }
            }
        }
    }

    void ContentBrowser::DrawSearchBar()
    {
        auto Size = std::max(ImGui::GetWindowWidth() / 5.f, 230.f);
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - Size - 10.f);
        ImGui::SetNextItemWidth(Size);
        if (ImGui::InputTextWithHint("##SearchBar", "(inc, -exc)", m_SearchFilter.InputBuf, IM_ARRAYSIZE(m_SearchFilter.InputBuf)))
            m_SearchFilter.Build();
    }

    void ContentBrowser::DrawItems()
    {
        constexpr int ViewSize = 96;
        const int     Columns  = std::clamp(int(ImGui::GetContentRegionAvail().x / ViewSize), 1, 64);

        imcxx::table Table{ "DirectoryInfo", Columns };
        if (!Table)
        {
            return;
        }

        auto ContentPackage = Project::Get()->GetContentPackage();

        for (auto& File : m_DirectoryIterator.GetAllFiles())
        {
            auto FileName = File.Path->stem().string();
            if (!m_SearchFilter.PassFilter(FileName.c_str(), FileName.c_str() + FileName.size()))
            {
                continue;
            }

            if (!Table.next_column())
            {
                continue;
            }

            auto Extension   = File.Path->has_extension() ? StringUtils::ToLower(File.Path->extension().string()) : "";
            auto TextureInfo = GetImageIcon(Extension, File.IsFile);

            ImVec4 IconTint(1.f, 1.f, 1.f, 1.f);
            if (File.IsFile)
            {
                if (!File.MetaData || ContentPackage->GetGuidOfPath(File.Path->string()).is_nil())
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

            bool ItemSelected = ImGui::IsItemHovered() &&
                                (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) ||
                                 ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)));

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ViewSize - ImGui::CalcTextSize(FileName.c_str()).x) / 2.0f);
            ImGui::TextWrapped(FileName.c_str());

            ItemSelected |= ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

            if (ItemSelected)
            {
                if (File.IsFile)
                {
                    // Open Asset
                }
                else
                {
                    m_DirectoryIterator.Visit(*File.Path);
                    return;
                }
            }
        }
    }
} // namespace Neon::Editor::Views