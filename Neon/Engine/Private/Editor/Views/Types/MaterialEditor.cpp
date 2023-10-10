#include <EnginePCH.hpp>
#include <Editor/Views/Types/MaterialEditor.hpp>

#include <UI/MaterialEditor.hpp>

namespace Neon::Editor::Views
{
    MaterialEditor::MaterialEditor(
        const StringU8& TabName) :
        IEditorView(StringUtils::Format("Material Editor: {}", TabName))
    {
        m_NodeEditor = UI::NodeEditor::EditorConfig{};
    }

    void MaterialEditor::OnRender()
    {
        imcxx::window Window(GetWidgetId(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
        if (!Window)
        {
            return;
        }

        auto Context = m_NodeEditor.Attach();
    }
} // namespace Neon::Editor::Views