#pragma once

#include <Core/Neon.hpp>
#include <UI/NodeEditor.hpp>

namespace Neon::UI
{
    class NodeBuilder
    {
    public:
        NodeBuilder(
            NodeEditor* Editor,
            ImTextureID Texture = nullptr,
            uint32_t    Width   = 0,
            uint32_t    Height  = 0);

        void Begin(
            NodeEditor::NodeId Id);
        void End();

        void Header(
            const ImColor& Color = ImColor(1.f, 1.f, 1.f, 1.f));
        void EndHeader();

        void Input(
            NodeEditor::PinId Id);
        void EndInput();

        void Middle();

        void Output(
            NodeEditor::PinId Id);
        void EndOutput();

    private:
        void Pin(
            NodeEditor::PinId       Id,
            ax::NodeEditor::PinKind Kind);

        void EndPin();

    private:
        NodeEditor* m_Editor;

        ImTextureID m_HeaderTextureId;
        uint32_t    m_HeaderTextureWidth;
        uint32_t    m_HeaderTextureHeight;

        NodeEditor::NodeId m_CurrentNodeId;

        ImVec2 m_StartPos{};
        ImVec2 m_EndPos{};
        ImU32  m_HeaderColor{};

        bool m_HasHeader : 1 = false;
    };
} // namespace Neon::UI