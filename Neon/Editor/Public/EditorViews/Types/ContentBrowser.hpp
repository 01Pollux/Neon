#pragma once

#include <EditorViews/View.hpp>

namespace Neon::Editor::Views
{
    class ContentBrowser : public IEditorView
    {
    public:
        void OnUpdate() override;

        void OnRender() override;
    };
} // namespace Neon::Editor::Views