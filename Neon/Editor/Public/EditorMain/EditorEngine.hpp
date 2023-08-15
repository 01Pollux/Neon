#pragma once

#include <Runtime/GameEngine.hpp>
#include <EditorViews/View.hpp>

#include <unordered_map>
#include <unordered_set>

namespace Neon::Editor
{
    using EditorViewId = uint64_t;

    class EditorEngine : public Runtime::GameEngine
    {
    public:
        /// <summary>
        /// Get the singleton instance of the editor.
        /// </summary>
        [[nodiscard]] static EditorEngine* Get()
        {
            return static_cast<EditorEngine*>(GameEngine::Get());
        }

        void Initialize(
            Config::EngineConfig Config) override;

    public:
        /// <summary>
        /// Register a new editor view.
        /// </summary>
        [[nodiscard]] EditorViewId RegisterView(
            UPtr<IEditorView> View);

        /// <summary>
        /// Register a new editor view.
        /// </summary>
        template<typename _Ty, typename... _Args>
            requires std::derived_from<_Ty, IEditorView>
        [[nodiscard]] EditorViewId RegisterView(
            _Args&&... Args)
        {
            return RegisterView(std::make_unique<_Ty>(std::forward<_Args>(Args)...));
        }

        /// <summary>
        /// Unregister an editor view.
        /// </summary>
        void UnregisterView(
            EditorViewId ViewId);

        /// <summary>
        /// Open the editor view.
        /// </summary>
        void OpenView(
            EditorViewId Id);

        /// <summary>
        /// Check if the editor view is open.
        /// </summary>
        bool IsViewOpen(
            EditorViewId ViewId);

        /// <summary>
        /// Close the editor view.
        /// </summary>
        void CloseView(
            EditorViewId ViewId);

    private:
        std::unordered_map<uint64_t, UPtr<IEditorView>> m_Views;

        std::unordered_set<IEditorView*> m_OpenViews;
    };
} // namespace Neon::Editor