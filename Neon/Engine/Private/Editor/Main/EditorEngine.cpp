#include <EnginePCH.hpp>
#include <Editor/Main/EditorEngine.hpp>
#include <Editor/Project/DefaultFonts.hpp>
#include <Editor/Profile/Manager.hpp>

#include <Window/Window.hpp>
#include <Asset/Packs/Directory.hpp>
#include <Asset/Storage.hpp>

#include <imgui/imgui.h>
#include <Log/Logger.hpp>

namespace Neon::Editor
{
    void EditorEngine::Initialize(
        Config::EditorConfig Config)
    {
        auto ContentPackage = std::make_unique<Asset::DirectoryAssetPackage>("Content");
        m_ContentPackage    = ContentPackage.get();

        Config.Resource.AssetPackages.emplace_back(std::make_unique<Asset::DirectoryAssetPackage>("Editor"));
        Config.Resource.AssetPackages.emplace_back(std::move(ContentPackage));

        auto StartupProjectPath = std::move(Config.StartupProjectPath);
        auto NewProjectName     = std::move(Config.NewProjectName);

        NEON_ASSERT(!StartupProjectPath.empty(), "No project path specified");

        GameEngine::Initialize(std::move(Config));

        GetWindow()->OnWindowTitleHitTest().Listen(
            [this](const Vector2I& MousePos, bool& WasHit)
            {
                WasHit = m_IsTitlebarHovered;
            });

        // Load the default fonts
        LoadDefaultFonts();

        // Load profile
        // TODO: Load from config
        ProfileManager::Load(Asset::Handle::FromString("1f212ba0-6313-4452-8dec-92b34f7b21e3"));

        // Register editor system
        AddStandardEditorSystem();

        // Register views
        AddStandardViews();

        //

        if (!NewProjectName.empty())
        {
            ProjectManager::Get()->NewEmptyProject(StartupProjectPath, { .Name = std::move(NewProjectName) });
        }
        else
        {
            if (!ProjectManager::Get()->OpenProject(StartupProjectPath))
            {
                NEON_WARNING("Editor", "Failed to open project: {}, Creating new one", StartupProjectPath);
                ProjectManager::Get()->NewEmptyProject(StartupProjectPath);
            }
        }
    }

    void EditorEngine::PreUpdate()
    {
        GameEngine::PreUpdate();
        for (auto& View : m_OpenViews)
        {
            View->OnUpdate();
        }
    }

    void EditorEngine::PostRender()
    {
        GameEngine::PostRender();

        if (BeginEditorSpace())
        {
            RenderMenuBar();
            for (auto& View : m_OpenViews)
            {
                View->OnRender();
            }
        }
        EndEditorSpace();
    }

    //

    void EditorEngine::RegisterView(
        const StringU8&   Name,
        UPtr<IEditorView> View,
        bool              InitialVisible)
    {
        auto ViewPtr = View.get();
        m_Views.emplace(Name, std::move(View));
        if (InitialVisible)
        {
            m_OpenViews.insert(ViewPtr);
        }
    }

    void EditorEngine::UnregisterView(
        const StringU8& Name)
    {
        auto Iter = m_Views.find(Name);
        if (Iter == m_Views.end())
        {
            NEON_WARNING("Editor", "Trying to unregister a view that does not exist.");
            return;
        }
        m_OpenViews.erase(Iter->second.get());
        m_Views.erase(Iter);
    }

    void EditorEngine::OpenView(
        const StringU8& Name)
    {
        auto Iter = m_Views.find(Name);
        if (Iter == m_Views.end())
        {
            NEON_WARNING("Editor", "Trying to open a view that does not exist.");
            return;
        }

        auto [View, ShouldOpen] = m_OpenViews.emplace(Iter->second.get());
        if (ShouldOpen)
        {
            Iter->second->OnOpen();
        }
    }

    bool EditorEngine::IsViewOpen(
        const StringU8& Name)
    {
        auto Iter = m_Views.find(Name);
        return Iter != m_Views.end() ? m_OpenViews.contains(Iter->second.get()) : false;
    }

    void EditorEngine::CloseView(
        const StringU8& Name)
    {
        auto Iter = m_Views.find(Name);
        if (Iter == m_Views.end())
        {
            NEON_WARNING("Editor", "Trying to close a view that does not exist.");
            return;
        }

        auto View = Iter->second.get();
        if (m_OpenViews.erase(View))
        {
            View->OnClose();
        }
    }
} // namespace Neon::Editor
