#pragma once

#include <Editor/Views/View.hpp>

namespace Neon::Editor::Views
{
    class SceneHierachy : public IEditorView
    {
    public:
        SceneHierachy();

        void OnRender() override;
    };
} // namespace Neon::Editor::Views