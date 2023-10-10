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
        m_HeaderMin = m_HeaderMax = ImVec2();

        m_Editor->PushStyleVar(AxNodeEditor::StyleVar_NodePadding, ImVec4(8, 4, 8, 8));
        m_Editor->BeginNode(Id);

        ImGui::PushID(Id.AsPointer());
        m_CurrentNodeId = Id;

        SetStage(Stage::Begin);
    }

    void NodeBuilder::End()
    {
        SetStage(Stage::End);

        m_Editor->EndNode();

        if (ImGui::IsItemVisible() && m_HeaderTextureId)
        {
            auto Alpha = static_cast<int>(255 * ImGui::GetStyle().Alpha);

            auto DrawList = m_Editor->GetNodeBackgroundDrawList(m_CurrentNodeId);

            const auto HalfBorderWidth = m_Editor->GetStyle().NodeBorderWidth * 0.5f;

            auto HeaderColor = IM_COL32(0, 0, 0, Alpha) | (m_HeaderColor & IM_COL32(255, 255, 255, 0));
            if ((m_HeaderMax.x > m_HeaderMin.x) && (m_HeaderMax.y > m_HeaderMin.y))
            {
                const auto uv = ImVec2(
                    (m_HeaderMax.x - m_HeaderMin.x) / (float)(4.0f * m_HeaderTextureWidth),
                    (m_HeaderMax.y - m_HeaderMin.y) / (float)(4.0f * m_HeaderTextureHeight));

                DrawList->AddImageRounded(m_HeaderTextureId,
                                          m_HeaderMin - ImVec2(8 - HalfBorderWidth, 4 - HalfBorderWidth),
                                          m_HeaderMax + ImVec2(8 - HalfBorderWidth, 0),
                                          ImVec2(0.0f, 0.0f), uv,
                                          HeaderColor, m_Editor->GetStyle().NodeRounding, ImDrawFlags_RoundCornersTop);

                if (m_ContentMin.y > m_HeaderMax.y)
                {
                    DrawList->AddLine(
                        ImVec2(m_HeaderMin.x - (8 - HalfBorderWidth), m_HeaderMax.y - 0.5f),
                        ImVec2(m_HeaderMax.x + (8 - HalfBorderWidth), m_HeaderMax.y - 0.5f),
                        ImColor(255, 255, 255, 96 * Alpha / (3 * 255)), 1.0f);
                }
            }
        }

        m_CurrentNodeId = 0;

        ImGui::PopID();

        m_Editor->PopStyleVar();

        SetStage(Stage::Invalid);
    }

    void NodeBuilder::Header(
        const ImColor& Color)
    {
        m_HeaderColor = Color;
        SetStage(Stage::Header);
    }

    void NodeBuilder::EndHeader()
    {
        SetStage(Stage::Content);
    }

    void NodeBuilder::Input(
        NodeEditor::PinId Id)
    {
        if (m_CurrentStage == Stage::Begin)
            SetStage(Stage::Content);

        SetStage(Stage::Input);

        Pin(Id, AxNodeEditor::PinKind::Input);
    }

    void NodeBuilder::EndInput()
    {
        EndPin();
    }

    void NodeBuilder::Middle()
    {
        if (m_CurrentStage == Stage::Begin)
            SetStage(Stage::Content);

        SetStage(Stage::Middle);
    }

    void NodeBuilder::Output(
        NodeEditor::PinId id)
    {
        if (m_CurrentStage == Stage::Begin)
            SetStage(Stage::Content);

        SetStage(Stage::Output);

        Pin(id, AxNodeEditor::PinKind::Output);
    }

    void NodeBuilder::EndOutput()
    {
        EndPin();
    }

    bool NodeBuilder::SetStage(
        Stage NewStage)
    {
        if (NewStage == m_CurrentStage)
            return false;

        auto oldStage  = m_CurrentStage;
        m_CurrentStage = NewStage;

        ImVec2 cursor;
        switch (oldStage)
        {
        case Stage::Header:
            m_HeaderMin = ImGui::GetItemRectMin();
            m_HeaderMax = ImGui::GetItemRectMax();
            break;

        case Stage::Input:
            m_Editor->PopStyleVar(2);
            break;

        case Stage::Output:
            m_Editor->PopStyleVar(2);
            break;
        }

        switch (NewStage)
        {
        case Stage::Header:
            m_HasHeader = true;
            break;

        case Stage::Input:
            m_Editor->PushStyleVar(AxNodeEditor::StyleVar_PivotAlignment, ImVec2(0, 0.5f));
            m_Editor->PushStyleVar(AxNodeEditor::StyleVar_PivotSize, ImVec2(0, 0));
            break;

        case Stage::Output:
            m_Editor->PushStyleVar(AxNodeEditor::StyleVar_PivotAlignment, ImVec2(1.0f, 0.5f));
            m_Editor->PushStyleVar(AxNodeEditor::StyleVar_PivotSize, ImVec2(0, 0));
            break;

        case Stage::End:
            m_NodeMin = ImGui::GetItemRectMin();
            m_NodeMax = ImGui::GetItemRectMax();
            break;

        case Stage::Invalid:
            break;
        }

        return true;
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