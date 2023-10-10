#pragma once

#include <Editor/Views/View.hpp>
#include <UI/NodeEditor.hpp>

namespace Neon::Editor::Views
{
    class MaterialEditor : public IEditorView
    {
    public:
        MaterialEditor(
            const StringU8& TabName);

        void OnRender() override;

    private:
        UI::NodeEditor m_NodeEditor;
    };
} // namespace Neon::Editor::Views