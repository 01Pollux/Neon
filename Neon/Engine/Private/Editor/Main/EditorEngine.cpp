#include <EnginePCH.hpp>
#include <Editor/Main/EditorEngine.hpp>
#include <Window/Window.hpp>

#include <Asset/Packs/Directory.hpp>

#include <imgui/imgui.h>
#include <Log/Logger.hpp>

namespace Neon::Editor
{
    void EditorEngine::Initialize(
        Config::EngineConfig Config)
    {
        Config.Resource.AssetPackages.emplace_back(std::make_unique<Asset::DirectoryAssetPackage>("Contents"));
        GameEngine::Initialize(std::move(Config));

        AddStandardViews();
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

        BeginEditorSpace();
        for (auto& View : m_OpenViews)
        {
            View->OnRender();
        }
        EndEditorSpace();
    }

    //

    void EditorEngine::RegisterView(
        const StringU8&   Name,
        UPtr<IEditorView> View)
    {
        m_Views.emplace(Name, std::move(View));
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
