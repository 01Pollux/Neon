#pragma once

#include <Editor/Views/View.hpp>

namespace Neon::Editor::Views
{
    class GameDisplay : public IEditorView
    {
    public:
        GameDisplay();

        void OnUpdate() override;

        void OnRender() override;
    };
} // namespace Neon::Editor::Views