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
            Pin  PinData;
            bool IsInput;
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
            Pin               NewPin);

        /// <summary>
        /// Remove pin from the node
        /// </summary>
        void RemovePin(
            NodeGraph::PinId TargetPin);

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
        void BeginNode(
            NodeGraph::NodeId Id);
        void EndNode();

        void Header(
            ImTextureID    Texture = nullptr,
            uint32_t       Width   = 0,
            uint32_t       Height  = 0,
            const ImColor& Color   = ImColor(255, 255, 255, 255));
        void EndHeader();

        void Input(
            NodeGraph::PinId Id);
        void EndInput();

        void Output(
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

        ImTextureID m_HeaderTextureId;
        uint32_t    m_HeaderTextureWidth;
        uint32_t    m_HeaderTextureHeight;

        NodeGraph::NodeId m_CurrentNodeId;

        ImVec2 m_StartPos{};
        ImVec2 m_EndPos{};
        ImU32  m_HeaderColor{};

        bool m_HasHeader : 1 = false;
    };
} // namespace Neon::UI::Graph