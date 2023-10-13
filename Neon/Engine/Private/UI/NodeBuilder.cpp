#include <EnginePCH.hpp>
#include <UI/Nodes/NodeBuilder.hpp>

namespace Neon::UI::Graph
{
    static Utils::BasicIconType GetBasicIconType(
        Pin::Type PinType)
    {
        switch (PinType)
        {
        case Pin::Type::Flow:
            return Utils::BasicIconType::Flow;
        case Pin::Type::Bool:
        case Pin::Type::Int:
        case Pin::Type::Float:
        case Pin::Type::String:
        case Pin::Type::Object:
        case Pin::Type::Function:
            return Utils::BasicIconType::Circle;
        case Pin::Type::Delegate:
            return Utils::BasicIconType::Square;
        default:
            return Utils::BasicIconType::Diamond;
        }
    }

    //

    bool Pin::AcceptLink(
        const Pin& Other) const
    {
        if (this == &Other)
        {
            UI::Utils::DrawLabel("x Incompatible Pin Kind", ImColor(45, 32, 32, 180));
        }
        else if (GetType() != Other.GetType())
        {
            UI::Utils::DrawLabel("x Incompatible Pin Type", ImColor(45, 32, 32, 180));
        }
        else
        {
            return true;
        }
        return false;
    }

    //

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
            m_Pins.erase(PinId);
        }
        m_Nodes.erase(Iter);
    }

    NodeGraph::PinId NodeBuilder::AddPin(
        NodeGraph::NodeId TargetNode,
        bool              IsInput,
        UPtr<Pin>         NewPin)
    {
        NodeGraph::PinId Id(m_NextId++);

        auto& NodeDesc = m_Nodes.at(TargetNode);
        m_Pins.emplace(Id, PinDescriptor{ .PinData = std::move(NewPin), .IsInput = IsInput });
        NodeDesc.Pins.emplace(Id);

        return Id;
    }

    NodeGraph::LinkId NodeBuilder::LinkPins(
        Link NewLink)
    {
        NodeGraph::LinkId Id(m_NextId++);

        auto& SrcPin = m_Pins.at(NewLink.GetSource());
        auto& DstPin = m_Pins.at(NewLink.GetDest());

        m_Links.emplace(Id, std::move(NewLink));
        SrcPin.LinkCount++;
        DstPin.LinkCount++;

        return Id;
    }

    void NodeBuilder::UnlinkPins(
        NodeGraph::LinkId TargetLink)
    {
        auto LinkIter = m_Links.find(TargetLink);

        auto SrcPin = m_Pins.find(LinkIter->second.GetSource());
        auto DstPin = m_Pins.find(LinkIter->second.GetDest());

        if (SrcPin != m_Pins.end())
        {
            SrcPin->second.LinkCount--;
        }
        if (DstPin != m_Pins.end())
        {
            DstPin->second.LinkCount--;
        }

        m_Links.erase(LinkIter);
    }

    //

    void NodeBuilder::Render()
    {
        RenderNodes();
        RenderCreator();
    }

    //

    void NodeBuilder::RenderNodes()
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
                        UI::Utils::DrawIcon(
                            { 24, 24 },
                            GetBasicIconType(PinDesc.PinData->GetType()),
                            PinDesc.LinkCount > 0,
                            PinDesc.PinData->GetColorFromType(),
                            ImColor(32.f, 32.f, 32.f, Alpha));
                        ImGui::SameLine();
                        imcxx::text PortName{ PinDesc.PinData->GetName() };
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
                            imcxx::text         PortName{ PinDesc.PinData->GetName() };
                            ImGui::SameLine();
                            UI::Utils::DrawIcon(
                                { 24, 24 },
                                GetBasicIconType(PinDesc.PinData->GetType()),
                                PinDesc.LinkCount > 0,
                                PinDesc.PinData->GetColorFromType(),
                                ImColor(32.f, 32.f, 32.f, Alpha));
                        }

                        EndOutput();
                    }
                }
            }

            EndNode();
        }

        for (auto& [LinkId, LinkData] : m_Links)
        {
            m_NodeGraph.Link(LinkId, LinkData.GetSource(), LinkData.GetDest(), LinkData.GetColor(), 2.0f);
        }
    }

    void NodeBuilder::RenderCreator()
    {
        if (m_CreateNewNode)
        {
            return;
        }

        if (m_NodeGraph.BeginCreate())
        {
            NodeGraph::PinId StartPinId = 0, EndPinId = 0;
            if (m_NodeGraph.QueryNewLink(&StartPinId, &EndPinId))
            {
                auto StartIter = m_Pins.find(StartPinId);
                auto EndIter   = m_Pins.find(EndPinId);

                bool FoundStart = StartIter != m_Pins.end();
                bool FoundEnd   = EndIter != m_Pins.end();

                m_NewLinkPin = FoundStart ? StartIter->first : EndIter->first;

                if (FoundStart && FoundEnd)
                {
                    // If the start is the input, swap them
                    if (StartIter->second.IsInput)
                    {
                        std::swap(StartIter, EndIter);
                    }

                    auto& StartPinDesc = StartIter->second;
                    auto& EndPinDesc   = EndIter->second;

                    //

                    if (StartIter == EndIter)
                    {
                        m_NodeGraph.RejectNewItem(ImColor(255, 0, 0), 2.0f);
                    }
                    if (!StartPinDesc.PinData->AcceptLink(*EndPinDesc.PinData))
                    {
                        m_NodeGraph.RejectNewItem(ImColor(255, 0, 0), 2.0f);
                    }
                    else
                    {
                        UI::Utils::DrawLabel("x Create Link", ImColor(32, 45, 32, 180));
                        if (m_NodeGraph.AcceptNewItem(ImColor(128, 255, 128), 4.0f))
                        {
                            LinkPins(Link(StartPinId, EndPinId, EndPinDesc.PinData->GetColorFromType()));
                        }
                    }
                }
            }

            NodeGraph::PinId PinId = 0;
            if (m_NodeGraph.QueryNewNode(&PinId))
            {
                auto NewLinkPin = m_Pins.find(PinId);
                if (NewLinkPin != m_Pins.end())
                {
                    UI::Utils::DrawLabel("+ Create Node", ImColor(32, 45, 32, 180));
                    m_NewLinkPin = NewLinkPin->first;
                }

                if (m_NodeGraph.AcceptNewItem())
                {
                    m_NewNodeLinkPin = PinId;
                    m_NewLinkPin     = {};
                    m_CreateNewNode  = true;

                    m_NodeGraph.Suspend();
                    ImGui::OpenPopup("Create New Node");
                    m_NodeGraph.Resume();
                }
            }
        }
        else
        {
            m_NewLinkPin = {};
        }
        m_NodeGraph.EndCreate();

        if (m_NodeGraph.BeginDelete())
        {
            NodeGraph::NodeId NodeId = 0;
            while (m_NodeGraph.QueryDeletedNode(&NodeId))
            {
                if (m_NodeGraph.AcceptDeletedItem())
                {
                    RemoveNode(NodeId);
                }
            }

            NodeGraph::LinkId LinkId = 0;
            while (m_NodeGraph.QueryDeletedLink(&LinkId))
            {
                if (m_NodeGraph.AcceptDeletedItem())
                {
                    UnlinkPins(LinkId);
                }
            }
        }
        m_NodeGraph.EndDelete();

        auto openPopupPosition = ImGui::GetMousePos();
        m_NodeGraph.Suspend();
        /* if (m_NodeGraph.ShowNodeContextMenu(&contextNodeId))
             ImGui::OpenPopup("Node Context Menu");
         else if (m_NodeGraph.ShowPinContextMenu(&contextPinId))
             ImGui::OpenPopup("Pin Context Menu");
         else if (m_NodeGraph.ShowLinkContextMenu(&contextLinkId))
             ImGui::OpenPopup("Link Context Menu");
         else*/
        if (m_NodeGraph.ShowBackgroundContextMenu())
        {
            m_NewNodeLinkPin = {};
            ImGui::OpenPopup("Create New Node");
        }
        m_NodeGraph.Resume();
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