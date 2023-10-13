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

        /// <summary>
        /// Get pin's color from type
        /// </summary>
        [[nodiscard]] ImColor GetColorFromType() const noexcept
        {
            switch (GetType())
            {
            case Type::Flow:
                return ImColor(255, 255, 255);
            case Type::Bool:
                return ImColor(220, 48, 48);
            case Type::Int:
                return ImColor(68, 201, 156);
            case Type::Float:
                return ImColor(147, 226, 74);
            case Type::String:
                return ImColor(124, 21, 153);
            case Type::Object:
                return ImColor(51, 150, 215);
            case Type::Function:
                return ImColor(218, 0, 183);
            case Type::Delegate:
                return ImColor(255, 48, 48);
            default:
                return ImColor(255, 255, 255);
            }
        }

    public:
        Pin(StringU8 Name,
            Type     PinType) :
            m_Name(std::move(Name)),
            m_Type(PinType)
        {
        }

        NEON_CLASS_NO_COPYMOVE(Pin);
        virtual ~Pin() = default;

        /// <summary>
        /// Check if pin can accept link
        /// </summary>
        [[nodiscard]] virtual bool AcceptLink(
            const Pin& Other) const;

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

        /// <summary>
        /// Get node's color
        /// </summary>
        [[nodiscard]] ImColor GetColor() const noexcept
        {
            return m_Color;
        }

        /// <summary>
        /// Set node's color
        /// </summary>
        [[nodiscard]] void GetColor(
            ImColor Color) noexcept
        {
            m_Color = Color;
        }

    public:
        Node(StringU8 Name,
             Type     NodeType,
             ImColor  Color = ImColor(48, 44, 51)) :
            m_Name(std::move(Name)),
            m_Type(NodeType)
        {
        }

        NEON_CLASS_NO_COPYMOVE(Node);
        virtual ~Node() = default;

    private:
        StringU8 m_Name;
        ImColor  m_Color;
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