#pragma once

#include <Core/Neon.hpp>
#include <UI/imcxx/all_in_one.hpp>
#include <ImGuiNodeEditor/imgui_node_editor.h>
#include <ImGuiNodeEditor/imgui_node_editor_internal.h>

namespace Neon::UI::Graph
{
    namespace AxNodeEditor = ax::NodeEditor;

    class NodeGraph
    {
    public:
        using EditorConfig  = AxNodeEditor::Config;
        using EditorContext = AxNodeEditor::Detail::EditorContext;

        using Style      = AxNodeEditor::Style;
        using StyleColor = AxNodeEditor::StyleColor;
        using StyleVar   = AxNodeEditor::StyleVar;

        using NodeId        = AxNodeEditor::NodeId;
        using PinId         = AxNodeEditor::PinId;
        using LinkId        = AxNodeEditor::LinkId;
        using PinKind       = AxNodeEditor::PinKind;
        using FlowDirection = AxNodeEditor::FlowDirection;

    public:
        NodeGraph() = default;
        NodeGraph(
            const EditorConfig& Config)
        {
            m_Editor.reset(NEON_NEW EditorContext(&Config));
        }

        [[nodiscard]] EditorContext* operator->() const
        {
            return m_Editor.get();
        }

        [[nodiscard]] EditorContext* Get() const
        {
            return m_Editor.get();
        }

    public:
        /// <summary>
        /// Get editor's config
        /// </summary>
        [[nodiscard]] const EditorConfig& GetConfig()
        {
            return m_Editor->GetConfig();
        }

    public:
        [[nodiscard]] Style& GetStyle()
        {
            return m_Editor->GetStyle();
        }

        [[nodiscard]] const char* GetStyleColorName(StyleColor ColorIndex)
        {
            return m_Editor->GetStyle().GetColorName(ColorIndex);
        }

        void PushStyleColor(StyleColor ColorIndex, const ImVec4& Color)
        {
            m_Editor->GetStyle().PushColor(ColorIndex, Color);
        }

        void PopStyleColor(int Count = 1)
        {
            m_Editor->GetStyle().PopColor(Count);
        }

        void PushStyleVar(StyleVar VarIndex, float Value)
        {
            m_Editor->GetStyle().PushVar(VarIndex, Value);
        }

        void PushStyleVar(StyleVar VarIndex, const ImVec2& Value)
        {
            m_Editor->GetStyle().PushVar(VarIndex, Value);
        }

        void PushStyleVar(StyleVar VarIndex, const ImVec4& Value)
        {
            m_Editor->GetStyle().PushVar(VarIndex, Value);
        }

        void PopStyleVar(int Count = 1)
        {
            m_Editor->GetStyle().PopVar(Count);
        }

        [[nodiscard]] bool Begin(const char* Id, const ImVec2& Size = ImVec2(0, 0))
        {
            return m_Editor->Begin(Id, Size);
        }

        void End()
        {
            m_Editor->End();
        }

        void BeginNode(NodeId Id)
        {
            m_Editor->GetNodeBuilder().Begin(Id);
        }

        void BeginPin(PinId Id, PinKind Kind)
        {
            m_Editor->GetNodeBuilder().BeginPin(Id, Kind);
        }

        void PinRect(const ImVec2& Min, const ImVec2& Max)
        {
            m_Editor->GetNodeBuilder().PinRect(Min, Max);
        }

        void PinPivotRect(const ImVec2& Min, const ImVec2& Max)
        {
            m_Editor->GetNodeBuilder().PinPivotRect(Min, Max);
        }

        void PinPivotSize(const ImVec2& Size)
        {
            m_Editor->GetNodeBuilder().PinPivotSize(Size);
        }

        void PinPivotScale(const ImVec2& Scale)
        {
            m_Editor->GetNodeBuilder().PinPivotScale(Scale);
        }

        void PinPivotAlignment(const ImVec2& Alignment)
        {
            m_Editor->GetNodeBuilder().PinPivotAlignment(Alignment);
        }

        void EndPin()
        {
            m_Editor->GetNodeBuilder().EndPin();
        }

        void Group(const ImVec2& Size)
        {
            m_Editor->GetNodeBuilder().Group(Size);
        }

        void EndNode()
        {
            m_Editor->GetNodeBuilder().End();
        }

        [[nodiscard]] bool BeginGroupHint(NodeId Id)
        {
            return m_Editor->GetHintBuilder().Begin(Id);
        }

        [[nodiscard]] ImVec2 GetGroupMin()
        {
            return m_Editor->GetHintBuilder().GetGroupMin();
        }

        [[nodiscard]] ImVec2 GetGroupMax()
        {
            return m_Editor->GetHintBuilder().GetGroupMax();
        }

        [[nodiscard]] ImDrawList* GetHintForegroundDrawList()
        {
            return m_Editor->GetHintBuilder().GetForegroundDrawList();
        }

        [[nodiscard]] ImDrawList* GetHintBackgroundDrawList()
        {
            return m_Editor->GetHintBuilder().GetBackgroundDrawList();
        }

        void EndGroupHint()
        {
            m_Editor->GetHintBuilder().End();
        }

        // TODO: Add Min way to manage node background channels
        [[nodiscard]] ImDrawList* GetNodeBackgroundDrawList(NodeId Id)
        {
            auto Node = m_Editor->FindNode(Id);
            return Node ? m_Editor->GetNodeBuilder().GetUserBackgroundDrawList(Node) : nullptr;
        }

        bool Link(LinkId Id, PinId StartPinId, PinId EndPinId, const ImColor& Color = ImColor(1, 1, 1, 1), float Thickness = 1.0f)
        {
            return m_Editor->DoLink(Id, StartPinId, EndPinId, Color, Thickness);
        }

        void Flow(LinkId Id, FlowDirection Direction = FlowDirection::Forward)
        {
            if (auto Link = m_Editor->FindLink(Id))
                m_Editor->Flow(Link, Direction);
        }

        [[nodiscard]] bool BeginCreate(const ImColor& Color = ImColor(255, 255, 255, 255), float Thickness = 1.5f)
        {
            auto& Context = m_Editor->GetItemCreator();

            if (Context.Begin())
            {
                Context.SetStyle(Color, Thickness);
                return true;
            }
            else
            {
                return false;
            }
        }

        [[nodiscard]] bool QueryNewLink(PinId* StartId, PinId* EndId)
        {
            using Result  = AxNodeEditor::Detail::CreateItemAction::Result;
            auto& Context = m_Editor->GetItemCreator();
            return Context.QueryLink(StartId, EndId) == Result::True;
        }

        [[nodiscard]] bool QueryNewLink(PinId* StartId, PinId* EndId, const ImColor& Color, float Thickness = 1.0f)
        {
            using Result  = AxNodeEditor::Detail::CreateItemAction::Result;
            auto& Context = m_Editor->GetItemCreator();

            auto result = Context.QueryLink(StartId, EndId);
            if (result != Result::Indeterminate)
                Context.SetStyle(Color, Thickness);

            return result == Result::True;
        }

        [[nodiscard]] bool QueryNewNode(PinId* Id)
        {
            using Result  = AxNodeEditor::Detail::CreateItemAction::Result;
            auto& Context = m_Editor->GetItemCreator();
            return Context.QueryNode(Id) == Result::True;
        }

        [[nodiscard]] bool QueryNewNode(PinId* Id, const ImColor& Color, float Thickness = 1.0f)
        {
            using Result  = AxNodeEditor::Detail::CreateItemAction::Result;
            auto& Context = m_Editor->GetItemCreator();

            auto result = Context.QueryNode(Id);
            if (result != Result::Indeterminate)
                Context.SetStyle(Color, Thickness);

            return result == Result::True;
        }

        [[nodiscard]] bool AcceptNewItem()
        {
            using Result  = AxNodeEditor::Detail::CreateItemAction::Result;
            auto& Context = m_Editor->GetItemCreator();
            return Context.AcceptItem() == Result::True;
        }

        [[nodiscard]] bool AcceptNewItem(const ImColor& Color, float Thickness = 1.0f)
        {
            using Result  = AxNodeEditor::Detail::CreateItemAction::Result;
            auto& Context = m_Editor->GetItemCreator();

            auto result = Context.AcceptItem();
            if (result != Result::Indeterminate)
                Context.SetStyle(Color, Thickness);

            return result == Result::True;
        }

        void RejectNewItem()
        {
            using Result  = AxNodeEditor::Detail::CreateItemAction::Result;
            auto& Context = m_Editor->GetItemCreator();
            Context.RejectItem();
        }

        void RejectNewItem(const ImColor& Color, float Thickness = 1.0f)
        {
            using Result  = AxNodeEditor::Detail::CreateItemAction::Result;
            auto& Context = m_Editor->GetItemCreator();

            auto result = Context.RejectItem();
            if (result != Result::Indeterminate)
                Context.SetStyle(Color, Thickness);
        }

        void EndCreate()
        {
            auto& Context = m_Editor->GetItemCreator();
            Context.End();
        }

        [[nodiscard]] bool BeginDelete()
        {
            auto& Context = m_Editor->GetItemDeleter();
            return Context.Begin();
        }

        [[nodiscard]] bool QueryDeletedLink(LinkId* Id, PinId* StartId = nullptr, PinId* EndId = nullptr)
        {
            auto& Context = m_Editor->GetItemDeleter();
            return Context.QueryLink(Id, StartId, EndId);
        }

        [[nodiscard]] bool QueryDeletedNode(NodeId* Id)
        {
            auto& Context = m_Editor->GetItemDeleter();
            return Context.QueryNode(Id);
        }

        [[nodiscard]] bool AcceptDeletedItem(bool DeleteDependencies = true)
        {
            auto& Context = m_Editor->GetItemDeleter();
            return Context.AcceptItem(DeleteDependencies);
        }

        void RejectDeletedItem()
        {
            auto& Context = m_Editor->GetItemDeleter();
            Context.RejectItem();
        }

        void EndDelete()
        {
            auto& Context = m_Editor->GetItemDeleter();
            Context.End();
        }

        void SetNodePosition(NodeId Id, const ImVec2& EditorPosition)
        {
            m_Editor->SetNodePosition(Id, EditorPosition);
        }

        void SetGroupSize(NodeId Id, const ImVec2& Size)
        {
            m_Editor->SetGroupSize(Id, Size);
        }

        [[nodiscard]] ImVec2 GetNodePosition(NodeId Id)
        {
            return m_Editor->GetNodePosition(Id);
        }

        [[nodiscard]] ImVec2 GetNodeSize(NodeId Id)
        {
            return m_Editor->GetNodeSize(Id);
        }

        void CenterNodeOnScreen(NodeId Id)
        {
            if (auto Node = m_Editor->FindNode(Id))
                Node->CenterOnScreenInNextFrame();
        }

        // Sets node z position, nodes with higher Value are drawn over nodes with lower Value
        void SetNodeZPosition(NodeId Id, float Z)
        {
            m_Editor->SetNodeZPosition(Id, Z);
        }

        // Returns node z position, defaults is 0.0f
        [[nodiscard]] float GetNodeZPosition(NodeId Id)
        {
            return m_Editor->GetNodeZPosition(Id);
        }

        void RestoreNodeState(NodeId Id)
        {
            if (auto Node = m_Editor->FindNode(Id))
                m_Editor->MarkNodeToRestoreState(Node);
        }

        void Suspend()
        {
            m_Editor->Suspend();
        }

        void Resume()
        {
            m_Editor->Resume();
        }

        [[nodiscard]] bool IsSuspended()
        {
            return m_Editor->IsSuspended();
        }

        [[nodiscard]] bool IsActive()
        {
            return m_Editor->IsFocused();
        }

        [[nodiscard]] bool HasSelectionChanged()
        {
            return m_Editor->HasSelectionChanged();
        }

        [[nodiscard]] auto& GetSelectedObject()
        {
            return m_Editor->GetSelectedObjects();
        }

        [[nodiscard]] auto GetSelectedNodes()
        {
            std::vector<NodeId> Nodes;
            for (auto& Node : GetSelectedObject())
            {
                if (Node->AsNode())
                {
                    Nodes.push_back(Node->ID().AsNodeId());
                }
            }
            return Nodes;
        }

        [[nodiscard]] auto GetSelectedLinks()
        {
            std::vector<LinkId> Links;
            for (auto& Link : GetSelectedObject())
            {
                if (Link->AsNode())
                {
                    Links.push_back(Link->ID().AsLinkId());
                }
            }
            return Links;
        }

        [[nodiscard]] bool IsNodeSelected(NodeId Id)
        {
            auto Node = m_Editor->FindNode(Id);
            return Node ? m_Editor->IsSelected(Node) : false;
        }

        [[nodiscard]] bool IsLinkSelected(LinkId Id)
        {
            auto Link = m_Editor->FindLink(Id);
            return Link ? m_Editor->IsSelected(Link) : false;
        }

        void ClearSelection()
        {
            m_Editor->ClearSelection();
        }

        void SelectNode(NodeId Id, bool Append = false)
        {
            if (auto Node = m_Editor->FindNode(Id))
            {
                if (Append)
                    m_Editor->SelectObject(Node);
                else
                    m_Editor->SetSelectedObject(Node);
            }
        }

        void SelectLink(LinkId Id, bool Append = false)
        {
            if (auto Link = m_Editor->FindLink(Id))
            {
                if (Append)
                    m_Editor->SelectObject(Link);
                else
                    m_Editor->SetSelectedObject(Link);
            }
        }

        void DeselectNode(NodeId Id)
        {
            if (auto Node = m_Editor->FindNode(Id))
                m_Editor->DeselectObject(Node);
        }

        void DeselectLink(LinkId Id)
        {
            if (auto Link = m_Editor->FindLink(Id))
                m_Editor->DeselectObject(Link);
        }

        bool DeleteNode(NodeId Id)
        {
            if (auto Node = m_Editor->FindNode(Id))
                m_Editor->GetItemDeleter().Add(Node);
        }

        bool DeleteLink(LinkId Id)
        {
            if (auto Link = m_Editor->FindLink(Id))
                m_Editor->GetItemDeleter().Add(Link);
        }

        // Returns true if node has any link connected
        [[nodiscard]] bool HasAnyLinks(NodeId Id)
        {
            return m_Editor->HasAnyLinks(Id);
        }

        // Return true if pin has any link connected
        [[nodiscard]] bool HasAnyLinks(PinId Id)
        {
            return m_Editor->HasAnyLinks(Id);
        }

        // Break all links connected to this node
        int BreakLinks(NodeId Id)
        {
            return m_Editor->BreakLinks(Id);
        }

        // Break all links connected to this pin
        int BreakLinks(PinId Id)
        {
            return m_Editor->BreakLinks(Id);
        }

        void NavigateToContent(float Duration = -1.f)
        {
            m_Editor->NavigateTo(m_Editor->GetContentBounds(), true, Duration);
        }

        void NavigateToSelection(bool ZoomIn = false, float Duration = -1)
        {
            m_Editor->NavigateTo(m_Editor->GetSelectionBounds(), ZoomIn, Duration);
        }

        [[nodiscard]] bool ShowNodeContextMenu(NodeId* Id)
        {
            return m_Editor->GetContextMenu().ShowNodeContextMenu(Id);
        }

        [[nodiscard]] bool ShowPinContextMenu(PinId* Id)
        {
            return m_Editor->GetContextMenu().ShowPinContextMenu(Id);
        }

        [[nodiscard]] bool ShowLinkContextMenu(LinkId* Id)
        {
            return m_Editor->GetContextMenu().ShowLinkContextMenu(Id);
        }

        [[nodiscard]] bool ShowBackgroundContextMenu()
        {
            return m_Editor->GetContextMenu().ShowBackgroundContextMenu();
        }

        void EnableShortcuts(bool Enable = true)
        {
            m_Editor->EnableShortcuts(Enable);
        }

        [[nodiscard]] bool AreShortcutsEnabled()
        {
            return m_Editor->AreShortcutsEnabled();
        }

        [[nodiscard]] bool BeginShortcut()
        {
            return m_Editor->GetShortcut().Begin();
        }

        [[nodiscard]] bool AcceptCut()
        {
            return m_Editor->GetShortcut().AcceptCut();
        }

        [[nodiscard]] bool AcceptCopy()
        {
            return m_Editor->GetShortcut().AcceptCopy();
        }

        [[nodiscard]] bool AcceptPaste()
        {
            return m_Editor->GetShortcut().AcceptPaste();
        }

        [[nodiscard]] bool AcceptDuplicate()
        {
            return m_Editor->GetShortcut().AcceptDuplicate();
        }

        [[nodiscard]] bool AcceptCreateNode()
        {
            return m_Editor->GetShortcut().AcceptCreateNode();
        }

        [[nodiscard]] const auto& GetActionContext()
        {
            return m_Editor->GetShortcut().m_Context;
        }

        [[nodiscard]] auto GetActionContextNodes()
        {
            std::vector<NodeId> Nodes;
            for (auto Node : GetActionContext())
            {
                if (Node->AsNode())
                {
                    Nodes.push_back(Node->ID().AsNodeId());
                }
            }
            return Nodes;
        }

        [[nodiscard]] auto GetActionContextLinks()
        {
            std::vector<LinkId> Links;
            for (auto Link : GetActionContext())
            {
                if (Link->AsNode())
                {
                    Links.push_back(Link->ID().AsLinkId());
                }
            }
            return Links;
        }

        void EndShortcut()
        {
            m_Editor->GetShortcut().End();
        }

        [[nodiscard]] float GetCurrentZoom()
        {
            return m_Editor->GetView().InvScale;
        }

        [[nodiscard]] NodeId GetHoveredNode()
        {
            return m_Editor->GetHoveredNode();
        }

        [[nodiscard]] PinId GetHoveredPin()
        {
            return m_Editor->GetHoveredPin();
        }

        [[nodiscard]] LinkId GetHoveredLink()
        {
            return m_Editor->GetHoveredLink();
        }

        [[nodiscard]] NodeId GetDoubleClickedNode()
        {
            return m_Editor->GetDoubleClickedNode();
        }

        [[nodiscard]] PinId GetDoubleClickedPin()
        {
            return m_Editor->GetDoubleClickedPin();
        }

        [[nodiscard]] LinkId GetDoubleClickedLink()
        {
            return m_Editor->GetDoubleClickedLink();
        }

        [[nodiscard]] bool IsBackgroundClicked()
        {
            return m_Editor->IsBackgroundClicked();
        }

        [[nodiscard]] bool IsBackgroundDoubleClicked()
        {
            return m_Editor->IsBackgroundDoubleClicked();
        }

        // -1 if none
        [[nodiscard]] ImGuiMouseButton GetBackgroundClickButtonIndex()
        {
            return m_Editor->GetBackgroundClickButtonIndex();
        }

        // -1 if none
        [[nodiscard]] ImGuiMouseButton GetBackgroundDoubleClickButtonIndex()
        {
            return m_Editor->GetBackgroundDoubleClickButtonIndex();
        }

        // pass nullptr if particular pin do not interest you
        [[nodiscard]] bool GetLinkPins(LinkId Id, PinId* StartPinId, PinId* EndPinId)
        {
            auto Link = m_Editor->FindLink(Id);
            if (!Link)
                return false;

            if (StartPinId)
                *StartPinId = Link->m_StartPin->m_ID;
            if (EndPinId)
                *EndPinId = Link->m_EndPin->m_ID;

            return true;
        }

        [[nodiscard]] bool PinHadAnyLinks(PinId Id)
        {
            return m_Editor->PinHadAnyLinks(Id);
        }

        [[nodiscard]] ImVec2 GetScreenSize()
        {
            return m_Editor->GetRect().GetSize();
        }

        [[nodiscard]] ImVec2 ScreenToCanvas(const ImVec2& Pos)
        {
            return m_Editor->ToCanvas(Pos);
        }

        [[nodiscard]] ImVec2 CanvasToScreen(const ImVec2& Pos)
        {
            return m_Editor->ToScreen(Pos);
        }

    private:
        std::unique_ptr<EditorContext> m_Editor;
    };
} // namespace Neon::UI::Graph