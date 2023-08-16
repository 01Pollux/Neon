#pragma once

#include <Editor/Views/View.hpp>

namespace Neon::Editor::Views
{
    class ContentBrowser : public IEditorView
    {
    public:
        ContentBrowser(
            const StringU8& Name);

        void OnUpdate() override;

        void OnRender() override;
    };
} // namespace Neon::Editor::Views