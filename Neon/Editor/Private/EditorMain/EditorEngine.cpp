#include <EditorPCH.hpp>
#include <EditorMain/EditorEngine.hpp>
#include <EditorViews/View.hpp>

#include <Asset/Packs/Directory.hpp>

#include <Log/Logger.hpp>

namespace Neon::Editor
{
    static EditorViewId s_EditorEngineId = 1;

    void EditorEngine::Initialize(
        Config::EngineConfig Config)
    {
        Config.Resource.AssetPackages.emplace_back(std::make_unique<Asset::DirectoryAssetPackage>("Contents"));
        GameEngine::Initialize(std::move(Config));
    }

    //

    EditorViewId EditorEngine::RegisterView(
        UPtr<IEditorView> View)
    {
        NEON_ASSERT(s_EditorEngineId, "Editor view id overflow.");

        uint64_t Id = s_EditorEngineId++;
        m_Views.emplace(Id, std::move(View));

        return Id;
    }

    void EditorEngine::UnregisterView(
        EditorViewId ViewId)
    {
        auto Iter = m_Views.find(ViewId);
        if (Iter == m_Views.end())
        {
            NEON_WARNING("Editor", "Trying to unregister a view that does not exist.");
            return;
        }
        m_OpenViews.erase(Iter->second.get());
        m_Views.erase(Iter);
    }

    void EditorEngine::OpenView(
        EditorViewId ViewId)
    {
        auto Iter = m_Views.find(ViewId);
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

    void EditorEngine::CloseView(
        EditorViewId ViewId)
    {
        auto Iter = m_Views.find(ViewId);
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
