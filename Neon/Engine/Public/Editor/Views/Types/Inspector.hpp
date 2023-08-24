#pragma once

#include <Editor/Views/View.hpp>

namespace Neon::Editor::Views
{
    class Inspector : public IEditorView
    {
    public:
        Inspector();

        void OnRender() override;
    };
} // namespace Neon::Editor::Views