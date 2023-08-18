#include <EnginePCH.hpp>
#include <Editor/Main/EditorEngine.hpp>
#include <Editor/Project/DefaultFonts.hpp>

#include <Window/Window.hpp>
#include <Asset/Packs/Directory.hpp>

#include <imgui/imgui.h>
#include <Log/Logger.hpp>

namespace Neon::Editor
{
    void EditorEngine::Initialize(
        Config::EngineConfig Config)
    {
        Config.Resource.AssetPackages.emplace_back(std::make_unique<Asset::DirectoryAssetPackage>("EditorData"));
        Config.Resource.AssetPackages.emplace_back(std::make_unique<Asset::DirectoryAssetPackage>("Content"));

        GameEngine::Initialize(std::move(Config));

        AddStandardViews();
        GetWindow()->OnWindowTitleHitTest().Listen(
            [this](const Vector2I& MousePos, bool& WasHit)
            {
                WasHit = m_IsTitlebarHovered;
            });

        // Load the default fonts
        LoadDefaultFonts();
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
