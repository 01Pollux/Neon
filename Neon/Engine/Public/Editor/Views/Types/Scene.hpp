#pragma once

#include <Editor/Views/View.hpp>

namespace Neon::Editor::Views
{
    class Scene : public IEditorView
    {
    public:
        Scene();

        void OnUpdate() override;

        void OnRender() override;
    };
} // namespace Neon::Editor::Views