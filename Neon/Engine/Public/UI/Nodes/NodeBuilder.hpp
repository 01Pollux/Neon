#pragma once

#include <Core/Neon.hpp>
#include <UI/Nodes/Node.hpp>

namespace Neon::UI::Graph
{
    class NodeBuilder
    {
    private:
        struct PinDescriptor
        {
            UPtr<Pin> PinData;
            uint32_t  LinkCount = 0;
            bool      IsInput;
        };

        struct NodeDescriptor
        {
            UPtr<Node>                 NodeData;
            std::set<NodeGraph::PinId> Pins;
        };

    public:
        NodeBuilder(
            NodeGraph::EditorConfig Config = {});

        [[nodiscard]] NodeGraph* operator->() noexcept
        {
            return &m_NodeGraph;
        }

        /// <summary>
        /// Add new node to the graph
        /// </summary>
        NodeGraph::NodeId AddNode(
            UPtr<Node> NewNode);

        /// <summary>
        /// Add new node to the graph
        /// </summary>
        template<typename _Ty, typename... _Args>
            requires std::is_base_of_v<Node, _Ty>
        NodeGraph::NodeId AddNode(
            _Args&&... Args)
        {
            return AddNode(std::make_unique<_Ty>(std::forward<_Args>(Args)...));
        }

        /// <summary>
        /// Remove node from the graph
        /// </summary>
        void RemoveNode(
            NodeGraph::NodeId TargetNode);

        /// <summary>
        /// Add new pin to the node
        /// </summary>
        NodeGraph::PinId AddPin(
            NodeGraph::NodeId TargetNode,
            bool              IsInput,
            UPtr<Pin>         NewPin);

        /// <summary>
        /// Add new pin to the node
        /// </summary>
        template<typename _Ty, typename... _Args>
            requires std::is_base_of_v<Pin, _Ty>
        NodeGraph::PinId AddPin(
            NodeGraph::NodeId TargetNode,
            bool              IsInput,
            _Args&&... Args)
        {
            return AddPin(TargetNode, IsInput, std::make_unique<_Ty>(std::forward<_Args>(Args)...));
        }

        /// <summary>
        /// Add link between pins
        /// </summary>
        NodeGraph::LinkId LinkPins(
            Link NewLink);

        /// <summary>
        /// Remove link between pins
        /// </summary>
        void UnlinkPins(
            NodeGraph::LinkId TargetLink);

    public:
        /// <summary>
        /// Render graph node
        /// </summary>
        void Render();

    private:
        /// <summary>
        /// Render graph nodes and their links
        /// </summary>
        void RenderNodes();

        /// <summary>
        /// Render popups and context menus
        /// </summary>
        void RenderCreator();

    private:
        void BeginNode(
            NodeGraph::NodeId Id);
        void EndNode();

        void BeginHeader(
            const ImColor& Color = ImColor(255, 255, 255, 255));
        void EndHeader();

        void BeginInput(
            NodeGraph::PinId Id);
        void EndInput();

        void BeginOutput(
            NodeGraph::PinId Id);
        void EndOutput();

    private:
        void BeginPin(
            NodeGraph::PinId   Id,
            NodeGraph::PinKind Kind);

        void EndPin();

    private:
        NodeGraph m_NodeGraph;
        uint64_t  m_NextId = 1;

        std::map<NodeGraph::NodeId, NodeDescriptor> m_Nodes;
        std::map<NodeGraph::PinId, PinDescriptor>   m_Pins;
        std::map<NodeGraph::LinkId, Link>           m_Links;

        NodeGraph::NodeId m_CurrentNodeId;
        NodeGraph::PinId  m_NewLinkPin{};
        NodeGraph::PinId  m_NewNodeLinkPin{};

        ImVec2 m_StartPos{};
        ImVec2 m_EndPos{};
        ImU32  m_HeaderColor{};

        bool m_HasHeader     : 1 = false;
        bool m_CreateNewNode : 1 = false;
    };
} // namespace Neon::UI::Graph