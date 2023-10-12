#include <EnginePCH.hpp>
#include <UI/Nodes/NodeBuilder.hpp>

namespace Neon::UI::Graph
{
    NodeBuilder::NodeBuilder(
        NodeGraph::EditorConfig Config) :
        m_NodeGraph(std::move(Config))
    {
    }

    NodeGraph::NodeId NodeBuilder::AddNode(
        UPtr<Node> NewNode)
    {
        NodeGraph::NodeId Id(m_NextId++);

        m_Nodes.emplace(Id, std::move(NewNode));

        return Id;
    }

    void NodeBuilder::RemoveNode(
        NodeGraph::NodeId TargetNode)
    {
        auto Iter = m_Nodes.find(TargetNode);
        for (auto& PinId : Iter->second.Pins)
        {
            for (auto LinkData = m_Links.begin(); LinkData != m_Links.end();)
            {
                if (LinkData->second.GetSource() == PinId ||
                    LinkData->second.GetDest() == PinId)
                {
                    LinkData = m_Links.erase(LinkData);
                }
                else
                {
                    LinkData++;
                }
            }

            m_Pins.erase(PinId);
        }
        m_Nodes.erase(Iter);
    }

    NodeGraph::PinId NodeBuilder::AddPin(
        NodeGraph::NodeId TargetNode,
        bool              IsInput,
        Pin               NewPin)
    {
        NodeGraph::PinId Id(m_NextId++);

        auto& NodeDesc = m_Nodes.at(TargetNode);
        m_Pins.emplace(Id, PinDescriptor{ std::move(NewPin), IsInput });
        NodeDesc.Pins.emplace(Id);

        return Id;
    }

    void NodeBuilder::RemovePin(
        NodeGraph::PinId TargetPin)
    {
        for (auto& CurNode : m_Nodes)
        {
            if (CurNode.second.Pins.erase(TargetPin))
            {
                for (auto LinkData = m_Links.begin(); LinkData != m_Links.end();)
                {
                    if (LinkData->second.GetSource() == TargetPin ||
                        LinkData->second.GetDest() == TargetPin)
                    {
                        LinkData = m_Links.erase(LinkData);
                    }
                    else
                    {
                        LinkData++;
                    }
                }

                m_Pins.erase(TargetPin);
                break;
            }
        }
    }

    NodeGraph::LinkId NodeBuilder::LinkPins(
        Link NewLink)
    {
        NodeGraph::LinkId Id(m_NextId++);

        m_Links.emplace(Id, std::move(NewLink));

        return Id;
    }

    void NodeBuilder::UnlinkPins(
        NodeGraph::LinkId TargetLink)
    {
        auto LinkIter = m_Links.find(TargetLink);
        m_Links.erase(LinkIter);
    }

    //

    void NodeBuilder::Render()
    {
    }

    //

    void NodeBuilder::BeginNode(
        NodeGraph::NodeId Id)
    {
        m_HasHeader = false;

        m_StartPos = {};
        m_EndPos   = {};

        m_NodeGraph.PushStyleVar(AxNodeEditor::StyleVar_NodePadding, ImVec4(8, 4, 8, 8));
        m_NodeGraph.BeginNode(Id);

        ImGui::PushID(Id.AsPointer());
        m_CurrentNodeId = Id;
    }

    void NodeBuilder::EndNode()
    {
        m_NodeGraph.EndNode();

        if (ImGui::IsItemVisible())
        {
            auto  DrawList        = m_NodeGraph.GetNodeBackgroundDrawList(m_CurrentNodeId);
            float HalfBorderWidth = m_NodeGraph.GetStyle().NodeBorderWidth * 0.5f;

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
                                              m_HeaderColor, m_NodeGraph.GetStyle().NodeRounding, ImDrawFlags_RoundCornersTop);
                }
                else
                {
                    DrawList->AddRectFilled(
                        MinPos,
                        MaxPos,
                        m_HeaderColor, m_NodeGraph.GetStyle().NodeRounding, ImDrawFlags_RoundCornersTop);
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

        m_NodeGraph.PopStyleVar();
    }

    void NodeBuilder::Header(
        ImTextureID    Texture,
        uint32_t       Width,
        uint32_t       Height,
        const ImColor& Color)
    {
        m_HeaderTextureId     = Texture;
        m_HeaderTextureWidth  = Width;
        m_HeaderTextureHeight = Height;
        m_HeaderColor         = Color;
        m_HasHeader           = true;

        ImGui::BeginGroup();
        m_StartPos = ImGui::GetCurrentWindow()->DC.CursorPos;
    }

    void NodeBuilder::EndHeader()
    {
        ImGui::EndGroup();
        m_EndPos = ImGui::GetCurrentWindow()->DC.CursorMaxPos;
    }

    void NodeBuilder::Input(
        NodeGraph::PinId Id)
    {
        m_NodeGraph.PushStyleVar(AxNodeEditor::StyleVar_PivotAlignment, ImVec2(0, 0.5f));
        m_NodeGraph.PushStyleVar(AxNodeEditor::StyleVar_PivotSize, ImVec2(0, 0));

        BeginPin(Id, AxNodeEditor::PinKind::Input);
    }

    void NodeBuilder::EndInput()
    {
        m_NodeGraph.PopStyleVar(2);
        EndPin();

        m_EndPos.x = std::max(m_EndPos.x, ImGui::GetCurrentWindow()->DC.CursorMaxPos.x);
    }

    void NodeBuilder::Output(
        NodeGraph::PinId id)
    {
        m_NodeGraph.PushStyleVar(AxNodeEditor::StyleVar_PivotAlignment, ImVec2(1.0f, 0.5f));
        m_NodeGraph.PushStyleVar(AxNodeEditor::StyleVar_PivotSize, ImVec2(0, 0));

        BeginPin(id, AxNodeEditor::PinKind::Output);
    }

    void NodeBuilder::EndOutput()
    {
        m_NodeGraph.PopStyleVar(2);
        EndPin();

        m_EndPos.x = std::max(m_EndPos.x, ImGui::GetCurrentWindow()->DC.CursorMaxPos.x);
    }

    void NodeBuilder::BeginPin(
        NodeGraph::PinId   Id,
        NodeGraph::PinKind Kind)
    {
        m_NodeGraph.BeginPin(Id, Kind);
    }

    void NodeBuilder::EndPin()
    {
        m_NodeGraph.EndPin();
    }
} // namespace Neon::UI::Graph