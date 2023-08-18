#pragma once

#include <Editor/Views/View.hpp>

namespace Neon::Editor::Views
{
    class Console : public IEditorView
    {
    public:
        Console();

        void OnUpdate() override;

        void OnRender() override;
    };
} // namespace Neon::Editor::Views