#pragma once

#include <Editor/Views/View.hpp>

namespace Neon::Editor::Views
{
    class Hierachy : public IEditorView
    {
    public:
        Hierachy();

        void OnUpdate() override;

        void OnRender() override;
    };
} // namespace Neon::Editor::Views