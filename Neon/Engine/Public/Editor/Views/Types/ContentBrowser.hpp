#pragma once

#include <Editor/Views/View.hpp>

#include <IO/FileWatcher.hpp>

namespace Neon::Editor::Views
{
    class ContentBrowserListener
        : public efsw::FileWatchListener
    {
    public:
        void handleFileAction(
            efsw::WatchID      WatchId,
            const std::string& dir,
            const std::string& FileName,
            efsw::Action       Action,
            std::string        OldFileName) override;
    };

    class ContentBrowser : public IEditorView
    {
    public:
        ContentBrowser();

        void OnUpdate() override;

        void OnRender() override;

    private:
        ContentBrowserListener m_Listener;
        efsw::FileWatcher      m_FileWatcher;
        efsw::WatchID          m_ContentID;
    };
} // namespace Neon::Editor::Views