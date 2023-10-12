#pragma once

#include <Editor/Views/View.hpp>
#include <UI/Nodes/NodeBuilder.hpp>

namespace Neon::Editor::Views
{
    class MaterialEditor : public IEditorView
    {
    public:
        MaterialEditor(
            const StringU8& TabName);

        void OnRender() override;

    private:
        UI::Graph::NodeBuilder m_NodeGraph;
    };
} // namespace Neon::Editor::Views