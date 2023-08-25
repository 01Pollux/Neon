#pragma once

#include <Editor/Views/View.hpp>

namespace Neon::Editor::Views
{
    class SceneDisplay : public IEditorView
    {
    public:
        SceneDisplay();

        void OnUpdate() override;

        void OnRender() override;
    };
} // namespace Neon::Editor::Views