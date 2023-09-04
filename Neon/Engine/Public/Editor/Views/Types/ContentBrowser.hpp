#pragma once

#include <Editor/Views/View.hpp>
#include <Editor/Views/Types/ContentBrowser/DirectoryIterator.hpp>

#include <IO/FileWatcher.hpp>

namespace Neon::Editor::Views
{
    class ContentBrowser : public IEditorView
    {
        class FileListener
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

    public:
        ContentBrowser();

        void OnRender() override;

    private:
        /// <summary>
        /// Get the image icon for the asset.
        /// </summary>
        [[nodiscard]] ImTextureRectInfo GetImageIcon(
            const StringU8& FileName,
            bool            IsFile);

        /// <summary>
        /// Display the popup if the user right clicks on an asset(s).
        /// </summary>
        void DisplayPopup();

    private:
        FileListener      m_Listener;
        efsw::FileWatcher m_FileWatcher{};
        efsw::WatchID     m_ContentID{};

        CB::DirectoryIterator m_DirectoryIterator;
    };
} // namespace Neon::Editor::Views