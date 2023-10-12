#pragma once

#include <Core/Neon.hpp>
#include <Core/String.hpp>
#include <UI/Nodes/Graph.hpp>

namespace Neon::UI::Graph
{
    class Pin
    {
        friend class NodeGraph;

    public:
        enum class Type : uint8_t
        {
            Flow,
            Bool,
            Int,
            Float,
            String,
            Object,
            Function,
            Delegate,
        };

    public:
        /// <summary>
        /// Get pin's name
        /// </summary>
        [[nodiscard]] const StringU8& GetName() const noexcept
        {
            return m_Name;
        }

        /// <summary>
        /// Get pin's type
        /// </summary>
        [[nodiscard]] Type GetType() const noexcept
        {
            return m_Type;
        }

        Pin(StringU8 Name,
            Type     PinType) :
            m_Name(std::move(Name)),
            m_Type(PinType)
        {
        }

    private:
        StringU8 m_Name;
        Type     m_Type;
    };

    class Node
    {
        friend class NodeGraph;

    public:
        enum class Type
        {
            Blueprint,
            Tree,
            Comment,
            Houdini
        };

    public:
        /// <summary>
        /// Get node's name
        /// </summary>
        [[nodiscard]] const StringU8& GetName() const noexcept
        {
            return m_Name;
        }

        /// <summary>
        /// Get node's type
        /// </summary>
        [[nodiscard]] Type GetType() const noexcept
        {
            return m_Type;
        }

    public:
        Node(StringU8 Name,
             Type     NodeType) :
            m_Name(std::move(Name)),
            m_Type(NodeType)
        {
        }

        NEON_CLASS_NO_COPYMOVE(Node);
        virtual ~Node() = default;

    private:
        StringU8 m_Name;
        Type     m_Type;
    };

    class Link
    {
        friend class NodeGraph;

    public:
        /// <summary>
        /// Get link's source
        /// </summary>
        [[nodiscard]] NodeGraph::PinId GetSource() const noexcept
        {
            return m_Start;
        }

        /// <summary>
        /// Get link's dest
        /// </summary>
        [[nodiscard]] NodeGraph::PinId GetDest() const noexcept
        {
            return m_Dest;
        }

        /// <summary>
        /// Get link's color
        /// </summary>
        [[nodiscard]] ImColor GetColor() const noexcept
        {
            return m_Color;
        }

        Link(NodeGraph::PinId Source,
             NodeGraph::PinId Dest,
             ImColor          Color) :
            m_Start(Source),
            m_Dest(Dest),
            m_Color(Color)
        {
        }

    private:
        NodeGraph::PinId m_Start{}, m_Dest{};
        ImColor          m_Color;
    };
} // namespace Neon::UI::Graph