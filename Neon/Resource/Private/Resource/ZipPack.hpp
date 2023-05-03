#pragma once

#include <Resource/Pack.hpp>
#include <Core/String.hpp>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/functional/hash.hpp>

namespace Neon::Asset
{
    class ZipAssetPack : public IAssetPack
    {
    public:
        struct PackInfo
        {
            /// <summary>
            /// Hash of the data in the mapped filed
            /// </summary>
            std::array<uint8_t, 32> Hash;

            /// <summary>
            /// Id of the loader to use
            /// </summary>
            size_t LoaderId = 0;

            /// <summary>
            /// Offset of data in mapped file
            /// </summary>
            size_t Offset = 0;

            /// <summary>
            /// Size of data in mapped file
            /// </summary>
            size_t Size = 0;
        };

        using LoadedAssetsMap = std::unordered_map<AssetHandle, Ptr<IAssetResource>, boost::hash<boost::uuids::uuid>>;
        using AssetInfoMap    = std::unordered_map<AssetHandle, PackInfo, boost::hash<boost::uuids::uuid>>;

    public:
        void Import(
            const StringU8& FilePath) override;

        void Export(
            const AssetResourceHandlers& Handlers,
            const StringU8&              FilePath) override;

        Ref<IAssetResource> Load(
            const AssetResourceHandlers& Handlers,
            const AssetHandle&           Handle) override;

        void Save(
            const AssetResourceHandlers& Handlers,
            const AssetHandle&           Handle,
            const Ptr<IAssetResource>&   Resource) override;

    private:
        Ref<IAssetResource> LoadAsset(
            const AssetResourceHandlers& Handlers,
            const AssetHandle&           Handle,
            StringU8&                    ErrorText);

    private:
        /// <summary>
        /// Open file as read only mode.
        /// </summary>
        void OpenFile(
            const StringU8& FilePath);

        /// <summary>
        /// Read file and validate if it contains valid header + valid data.
        /// </summary>
        bool ReadFile();

        /// <summary>
        /// Read sections to the header.
        /// </summary>
        bool Header_ReadSections();

        /// <summary>
        /// Read body to the file.
        /// </summary>
        bool Header_ReadBody();

    private:
        /// <summary>
        /// Write to file header and data.
        /// </summary>
        void WriteFile(
            const AssetResourceHandlers& Handlers);

        /// <summary>
        /// Write sections to the header.
        /// </summary>
        void Header_WriteSections();

        /// <summary>
        /// Write body to the header.
        /// </summary>
        void Header_WriteBody(
            const AssetResourceHandlers& Handlers);

    private:
        boost::iostreams::mapped_file m_FileView;

        LoadedAssetsMap m_LoadedAssets;
        AssetInfoMap    m_AssetsInfo;
    };
} // namespace Neon::Asset