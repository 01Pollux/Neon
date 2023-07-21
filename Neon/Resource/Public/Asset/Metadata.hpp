#pragma once

#include <boost/property_tree/ptree.hpp>
#include <Asset/Handle.hpp>

namespace Neon::AAsset
{
    struct AssetMetaData
    {
        AssetMetaData(
            boost::property_tree::ptree& MetaData);

        /// <summary>
        /// Get the asset's GUID.
        /// </summary>
        [[nodiscard]] Handle GetGuid() const noexcept;

        /// <summary>
        /// Get the asset's hash.
        /// </summary>
        [[nodiscard]] StringU8 GetHash() const noexcept;

        /// <summary>
        /// Get the asset's hash.
        /// </summary>
        [[nodiscard]] boost::property_tree::ptree& GetLoaderData() noexcept;

    private:
        boost::property_tree::ptree& m_MetaData;
        boost::property_tree::ptree* m_LoaderData = nullptr;
    };
} // namespace Neon::AAsset