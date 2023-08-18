#pragma once

#include <Editor/Views/View.hpp>

namespace Neon::Editor::Views
{
    class Game : public IEditorView
    {
    public:
        Game();

        void OnUpdate() override;

        void OnRender() override;
    };
} // namespace Neon::Editor::Views