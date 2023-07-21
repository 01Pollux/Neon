#pragma once

#include <boost/property_tree/ptree.hpp>
#include <Asset/Handle.hpp>

namespace Neon::AAsset
{
    using AssetMetaData = boost::property_tree::ptree;

    struct AssetMetaDataDef
    {
        AssetMetaDataDef(
            boost::property_tree::ptree MetaData);

        AssetMetaDataDef();

        /// <summary>
        /// Get the asset's GUID.
        /// </summary>
        [[nodiscard]] Handle GetGuid() const noexcept;

        /// <summary>
        /// Set the asset's GUID.
        /// </summary>
        void SetGuid(
            const Handle& Guid) noexcept;

        /// <summary>
        /// Get the asset's hash.
        /// </summary>
        [[nodiscard]] StringU8 GetHash() const noexcept;

        /// <summary>
        /// Get the asset's hash.
        /// </summary>
        void SetHash(
            StringU8 Hash) noexcept;

        /// <summary>
        /// Get the asset's hash.
        /// </summary>
        [[nodiscard]] AssetMetaData& GetLoaderData() noexcept;

        /// <summary>
        /// Get the asset's hash.
        /// </summary>
        [[nodiscard]] const AssetMetaData& GetLoaderData() const noexcept;

    private:
        AssetMetaData  m_MetaData;
        AssetMetaData* m_LoaderData = nullptr;
        bool           m_IsDirty    = false;
    };
} // namespace Neon::AAsset