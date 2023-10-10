#pragma once

#include <Core/Neon.hpp>
#include <UI/imcxx/all_in_one.hpp>
#include <ImGuiNodeEditor/imgui_node_editor.h>
#include <ImGuiNodeEditor/imgui_node_editor_internal.h>

namespace Neon::UI
{
    class NodeEditor
    {
    public:
        using EditorConfig = ax::NodeEditor::Config;

        NodeEditor() = default;
        NodeEditor(
            const EditorConfig& Config)
        {
            m_Editor = ax::NodeEditor::CreateEditor(&Config);
        }

        NEON_CLASS_NO_COPY(NodeEditor);

        NodeEditor(NodeEditor&& Other) :
            m_Editor(Other.m_Editor)
        {
            Other.m_Editor = nullptr;
        }

        NodeEditor& operator=(NodeEditor&& Other)
        {
            if (this != &Other)
            {
                this->~NodeEditor();
                m_Editor = std::exchange(Other.m_Editor, nullptr);
            }
            return *this;
        }

        ~NodeEditor()
        {
            if (m_Editor)
            {
                delete m_Editor;
                m_Editor = nullptr;
            }
        }

        [[nodiscard]] ax::NodeEditor::EditorContext* operator->() const
        {
            return m_Editor;
        }

        [[nodiscard]] ax::NodeEditor::EditorContext* Get() const
        {
            return m_Editor;
        }

    private:
        ax::NodeEditor::EditorContext* m_Editor = nullptr;
    };
} // namespace Neon::UI