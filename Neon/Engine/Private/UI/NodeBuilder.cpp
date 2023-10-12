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
        float  TextHeight = ImGui::GetTextLineHeight();
        ImVec2 HeaderPad{ 0.f, TextHeight };
        HeaderPad /= 2.f;

        for (auto& [Id, Node] : m_Nodes)
        {
            BeginNode(Id);

            // Draw header
            {
                BeginHeader(Node.NodeData->GetColor());

                ImGui::Dummy(HeaderPad);
                ImGui::SameLine();

                float       OldScale = UI::Utils::PushFontScaleMul(2.f);
                imcxx::text NodeName{ Node.NodeData->GetName() };
                UI::Utils::PopFontScale(OldScale);

                ImGui::Dummy(ImVec2(0, HeaderPad.y));

                EndHeader();
            }

            // Draw input ports
            bool HasOutput = false;
            {
                imcxx::group LockHorz;
                for (auto& Input : Node.Pins)
                {
                    auto& PinDesc = m_Pins.at(Input);
                    if (!PinDesc.IsInput)
                    {
                        HasOutput = true;
                        continue;
                    }

                    float Alpha = ImGui::GetStyle().Alpha;
                    /*   if (newLinkPin && !CanCreateLink(newLinkPin, &Input) && &Input != newLinkPin)
                           alpha *= (48.0f / 255.0f);*/

                    BeginInput(Input);

                    {

                        imcxx::shared_style AlphaOverride{ ImGuiStyleVar_Alpha, Alpha };
                        UI::Utils::DrawIcon({ 24, 24 }, UI::Utils::BasicIconType::Circle, false, ImColor(220, 48, 48), ImColor(32.f, 32.f, 32.f, Alpha));
                        ImGui::SameLine();
                        imcxx::text PortName{ PinDesc.PinData.GetName() };
                    }

                    EndInput();
                }
            }

            if (HasOutput)
            {
                ImGui::SameLine(0.f, 35.f);
            }

            // Draw output ports
            {
                imcxx::group LockHorz;
                {
                    for (auto& Output : Node.Pins)
                    {
                        auto& PinDesc = m_Pins.at(Output);
                        if (PinDesc.IsInput)
                        {
                            continue;
                        }

                        float Alpha = ImGui::GetStyle().Alpha;
                        /*   if (newLinkPin && !CanCreateLink(newLinkPin, &Input) && &Input != newLinkPin)
                               alpha *= (48.0f / 255.0f);*/

                        BeginOutput(Output);

                        {
                            imcxx::shared_style AlphaOverride{ ImGuiStyleVar_Alpha, Alpha };
                            imcxx::text         PortName{ PinDesc.PinData.GetName() };
                            ImGui::SameLine();
                            UI::Utils::DrawIcon({ 24, 24 }, UI::Utils::BasicIconType::Circle, false, ImColor(220, 48, 48), ImColor(32.f, 32.f, 32.f, Alpha));
                        }

                        EndOutput();
                    }
                }
            }

            EndNode();
        }
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

                DrawList->AddRectFilled(
                    MinPos,
                    MaxPos,
                    m_HeaderColor, m_NodeGraph.GetStyle().NodeRounding, ImDrawFlags_RoundCornersTop);

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

    void NodeBuilder::BeginHeader(
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

    void NodeBuilder::BeginInput(
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

    void NodeBuilder::BeginOutput(
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