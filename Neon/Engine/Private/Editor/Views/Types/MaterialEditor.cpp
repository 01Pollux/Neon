#include <EnginePCH.hpp>
#include <Editor/Views/Types/MaterialEditor.hpp>

//

namespace Neon::Editor::Views
{
    MaterialEditor::MaterialEditor(
        const StringU8& TabName) :
        IEditorView(StringUtils::Format("Material Editor: {}", TabName))
    {
    }

    void MaterialEditor::OnRender()
    {
        imcxx::window Window(GetWidgetId(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
        if (!Window)
        {
            return;
        }

        if (m_NodeGraph->Begin("Material Editor"))
        {
            m_NodeGraph.Render();
        }
        m_NodeGraph->End();
    }
} // namespace Neon::Editor::Views