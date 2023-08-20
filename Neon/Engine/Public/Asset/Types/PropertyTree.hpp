#pragma once

#include <Asset/Asset.hpp>
#include <boost/property_tree/ptree.hpp>

namespace Neon::Asset
{
    class PropertyTreeAsset : public IAsset
    {
    public:
        enum class TreeType : uint8_t
        {
            Json,
            Xml,
            Ini
        };

        class Handler;

        using IAsset::IAsset;

        /// <summary>
        /// Set property tree.
        /// </summary>
        void Set(
            boost::property_tree::ptree Tree);

        /// <summary>
        /// Get property tree.
        /// </summary>
        auto& Get() const noexcept
        {
            return m_Tree;
        }

        /// <summary>
        /// Set property tree type.
        /// </summary>
        void SetType(
            TreeType Type) noexcept
        {
            m_Type = Type;
        }

        /// <summary>
        /// Get property tree type.
        /// </summary>
        [[nodiscard]] TreeType GetType() const noexcept
        {
            return m_Type;
        }

    private:
        boost::property_tree::ptree m_Tree;
        TreeType                    m_Type;
    };
} // namespace Neon::Asset