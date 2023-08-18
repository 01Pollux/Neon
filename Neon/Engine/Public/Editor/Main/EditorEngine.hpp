#pragma once

#include <Runtime/GameEngine.hpp>
#include <Editor/Views/View.hpp>

#include <Editor/Project/Fonts/FontAwesome5.hpp>

#include <unordered_map>
#include <unordered_set>

namespace Neon::Editor
{
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

        void PreUpdate() override;

        void PostRender() override;

    public:
        /// <summary>
        /// Register a new editor view.
        /// </summary>
        [[nodiscard]] void RegisterView(
            const StringU8&   Name,
            UPtr<IEditorView> View);

        /// <summary>
        /// Register a new editor view.
        /// </summary>
        template<typename _Ty, typename... _Args>
            requires std::derived_from<_Ty, IEditorView>
        [[nodiscard]] void RegisterView(
            const StringU8& Name,
            _Args&&... Args)
        {
            RegisterView(Name, std::make_unique<_Ty>(std::forward<_Args>(Args)...));
        }

        /// <summary>
        /// Unregister an editor view.
        /// </summary>
        void UnregisterView(
            const StringU8& Name);

        /// <summary>
        /// Open the editor view.
        /// </summary>
        void OpenView(
            const StringU8& Name);

        /// <summary>
        /// Check if the editor view is open.
        /// </summary>
        bool IsViewOpen(
            const StringU8& Name);

        /// <summary>
        /// Close the editor view.
        /// </summary>
        void CloseView(
            const StringU8& Name);

    public:
        /// <summary>
        /// Add the standard views to the editor.
        /// </summary>
        void AddStandardViews();

        /// <summary>
        /// Begin the editor space by initializing dockspace.
        /// </summary>
        bool BeginEditorSpace();

        /// <summary>
        /// Render the menu bar.
        /// </summary>
        void RenderMenuBar();

        /// <summary>
        /// End the editor space by finalizing dockspace.
        /// </summary>
        void EndEditorSpace();

    private:
        std::unordered_map<StringU8, UPtr<IEditorView>> m_Views;

        std::unordered_set<IEditorView*> m_OpenViews;

        bool m_IsTitlebarHovered = false;
    };
} // namespace Neon::Editor