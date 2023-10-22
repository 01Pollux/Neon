#pragma once

#include <Editor/Views/View.hpp>

namespace Neon::Editor::Views
{
    class SceneDisplay : public IEditorView
    {
    public:
        SceneDisplay(
            bool IsEditorView = true);

        void OnUpdate() override;

        void OnRender() override;

    private:
        /// <summary>
        /// Get current camera to display
        /// </summary>
        [[nodiscard]] flecs::entity GetCurrentCamera();

    private:
        bool m_IsEditorView;
    };
} // namespace Neon::Editor::Views