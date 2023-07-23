#pragma once

#include <Asio/Coroutines.hpp>
#include <Asset/Handle.hpp>
#include <boost/property_tree/ptree.hpp>
#include <filesystem>

namespace Neon::Asset
{
    using AssetMetaData = boost::property_tree::ptree;

    struct AssetMetaDataDef
    {
        /// <summary>
        /// Creating an asset's metadata from an input stream.
        /// </summary>
        AssetMetaDataDef(
            std::ifstream& Stream);

        /// <summary>
        /// Creating an empty asset's metadata.
        /// </summary>
        AssetMetaDataDef(
            const Handle& AssetGuid,
            StringU8      Path);

        /// <summary>
        /// Export the asset's metadata.
        /// </summary>
        void Export(
            std::ofstream& Stream);

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
        /// Get the asset's loader id.
        /// </summary>
        [[nodiscard]] size_t GetLoaderId() const noexcept;

        /// <summary>
        /// Set the asset's loader id.
        /// </summary>
        [[nodiscard]] void SetLoaderId(
            size_t Id) noexcept;

        /// <summary>
        /// Get the asset's loader data.
        /// </summary>
        [[nodiscard]] AssetMetaData& GetLoaderData() noexcept;

        /// <summary>
        /// Get the asset's loader data.
        /// </summary>
        [[nodiscard]] const AssetMetaData& GetLoaderData() const noexcept;

        /// <summary>
        /// Get the asset's path.
        /// </summary>
        [[nodiscard]] std::filesystem::path GetAssetPath() const;

        /// <summary>
        /// Get the asset's metadata path.
        /// </summary>
        [[nodiscard]] StringU8 GetPath() const;

        /// <summary>
        /// Set the asset's metadata path.
        /// </summary>
        void SetPath(
            StringU8 Path);

        /// <summary>
        /// Query if the asset is dirty.
        /// </summary>
        [[nodiscard]] bool IsDirty() const noexcept;

        /// <summary>
        /// Set the asset's dirty state.
        /// </summary>
        void SetDirty(
            bool IsDirty = true) noexcept;

        /// <summary>
        /// Get the asset's dependencies.
        /// </summary>
        [[nodiscard]] Asio::CoGenerator<Handle> GetDependencies() const;

        /// <summary>
        /// Set the asset's dependencies.
        /// </summary>
        void SetDependencies(
            std::list<StringU8> Dependencies);

    private:
        AssetMetaData m_MetaData;
        bool          m_IsDirty = false;
    };
} // namespace Neon::Asset