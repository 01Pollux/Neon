#include <EnginePCH.hpp>
#include <UI/NodeBuilder.hpp>

namespace Neon::UI
{
    NodeBuilder::NodeBuilder(
        NodeEditor* Editor,
        ImTextureID Texture,
        uint32_t    Width,
        uint32_t    Height) :
        m_Editor(Editor),
        m_HeaderTextureId(Texture),
        m_HeaderTextureWidth(Width),
        m_HeaderTextureHeight(Height)
    {
    }

    void NodeBuilder::Begin(
        NodeEditor::NodeId Id)
    {
        m_HasHeader = false;

        // ImGui get absolute position of current cusror position
        // (relative to the top left corner of the current window)
        m_StartPos = {};
        m_EndPos   = {};

        m_Editor->PushStyleVar(AxNodeEditor::StyleVar_NodePadding, ImVec4(8, 4, 8, 8));
        m_Editor->BeginNode(Id);

        ImGui::PushID(Id.AsPointer());
        m_CurrentNodeId = Id;
    }

    void NodeBuilder::End()
    {
        m_Editor->EndNode();

        if (ImGui::IsItemVisible())
        {
            auto  DrawList        = m_Editor->GetNodeBackgroundDrawList(m_CurrentNodeId);
            float HalfBorderWidth = m_Editor->GetStyle().NodeBorderWidth * 0.5f;

            if (m_HasHeader)
            {
                ImVec2 MinPos = m_StartPos - ImVec2(8 - HalfBorderWidth, 4 - HalfBorderWidth);
                ImVec2 MaxPos = m_EndPos + ImVec2(8 - HalfBorderWidth, 0);

                if (m_HeaderTextureId)
                {
                    ImVec2 UV{
                        (m_EndPos.x - m_StartPos.x) / m_HeaderTextureWidth,
                        (m_EndPos.y - m_StartPos.y) / m_HeaderTextureHeight
                    };

                    DrawList->AddImageRounded(m_HeaderTextureId,
                                              MinPos,
                                              MaxPos,
                                              ImVec2(0.0f, 0.0f),
                                              UV,
                                              m_HeaderColor, m_Editor->GetStyle().NodeRounding, ImDrawFlags_RoundCornersTop);
                }
                else
                {
                    DrawList->AddRectFilled(
                        MinPos,
                        MaxPos,
                        m_HeaderColor, m_Editor->GetStyle().NodeRounding, ImDrawFlags_RoundCornersTop);
                }

                uint32_t Alpha = (m_HeaderColor & IM_COL32_A_MASK) >> IM_COL32_A_SHIFT;
                DrawList->AddLine(
                    ImVec2(MinPos.x - 0.5f, MaxPos.y - 0.5f),
                    ImVec2(MaxPos.x - 0.5f, MaxPos.y - 0.5f),
                    ImColor(255, 255, 255, 96 * Alpha / (3 * 255)), 1.0f);
            }
        }

        m_CurrentNodeId = 0;

        ImGui::PopID();

        m_Editor->PopStyleVar();
    }

    void NodeBuilder::Header(
        const ImColor& Color)
    {
        m_HeaderColor = Color;
        m_HasHeader   = true;

        ImGui::BeginGroup();
        m_StartPos = ImGui::GetCurrentWindow()->DC.CursorPos;
    }

    void NodeBuilder::EndHeader()
    {
        ImGui::EndGroup();
        m_EndPos = ImGui::GetCurrentWindow()->DC.CursorMaxPos;
    }

    void NodeBuilder::Input(
        NodeEditor::PinId Id)
    {
        m_Editor->PushStyleVar(AxNodeEditor::StyleVar_PivotAlignment, ImVec2(0, 0.5f));
        m_Editor->PushStyleVar(AxNodeEditor::StyleVar_PivotSize, ImVec2(0, 0));

        Pin(Id, AxNodeEditor::PinKind::Input);
    }

    void NodeBuilder::EndInput()
    {
        m_Editor->PopStyleVar(2);
        EndPin();

        m_EndPos.x = std::max(m_EndPos.x, ImGui::GetCurrentWindow()->DC.CursorMaxPos.x);
    }

    void NodeBuilder::Middle()
    {
    }

    void NodeBuilder::Output(
        NodeEditor::PinId id)
    {
        m_Editor->PushStyleVar(AxNodeEditor::StyleVar_PivotAlignment, ImVec2(1.0f, 0.5f));
        m_Editor->PushStyleVar(AxNodeEditor::StyleVar_PivotSize, ImVec2(0, 0));

        Pin(id, AxNodeEditor::PinKind::Output);
    }

    void NodeBuilder::EndOutput()
    {
        m_Editor->PopStyleVar(2);
        EndPin();

        m_EndPos.x = std::max(m_EndPos.x, ImGui::GetCurrentWindow()->DC.CursorMaxPos.x);
    }

    void NodeBuilder::Pin(
        NodeEditor::PinId   Id,
        NodeEditor::PinKind Kind)
    {
        m_Editor->BeginPin(Id, Kind);
    }

    void NodeBuilder::EndPin()
    {
        m_Editor->EndPin();
    }
} // namespace Neon::UI