#pragma once

#include <Resource/Pack.hpp>
#include <Core/String.hpp>
#include <Core/SHA256.hpp>

#include <fstream>
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
        ZipAssetPack(
            const AssetResourceHandlers& Handlers);

        ~ZipAssetPack() override;

    public:
        void Import(
            const StringU8& FilePath) override;

        void Export(
            const StringU8& FilePath) override;

        Ref<IAssetResource> Load(
            const AssetHandle& Handle) override;

        void Save(
            const AssetHandle&         Handle,
            const Ptr<IAssetResource>& Resource) override;

    private:
        Ref<IAssetResource> LoadAsset(
            const AssetResourceHandlers& Handlers,
            const AssetHandle&           Handle,
            StringU8&                    ErrorText);

    private:
        /// <summary>
        /// Get file name in temp folder.
        /// </summary>
        [[nodiscard]] StringU8 GetTempFileName() const;

        /// <summary>
        /// Truncate existing file (if it exists) and open new one.
        /// </summary>
        void OpenTempNew();

        /// <summary>
        /// Copy and decompress file into temp folder.
        /// </summary>
        void DecompressCopy(
            const StringU8& FilePath);

        /// <summary>
        /// Copy and decompress file into temp folder.
        /// </summary>
        void CompressCopy(
            const StringU8& FilePath);

    private:
        /// <summary>
        /// Get size of header + sections in file
        /// </summary>
        [[nodiscard]] static size_t SizeOfHeader(
            size_t NumberOfSections);

        /// <summary>
        /// Read file and validate if it contains valid header + valid data.
        /// </summary>
        bool ReadFile();

        /// <summary>
        /// Read file header to the header.
        /// </summary>
        bool Header_ReadHeader(
            struct AssetPackHeader& HeaderInfo);

        /// <summary>
        /// Read sections to the header.
        /// </summary>
        bool Header_ReadSections(
            SHA256&                       Header,
            const struct AssetPackHeader& HeaderInfo);

        /// <summary>
        /// Read body to the file.
        /// </summary>
        void Header_ReadBody();

    private:
        /// <summary>
        /// Write to file header and data.
        /// </summary>
        void WriteFile();

        /// <summary>
        /// Write header information to the header.
        /// </summary>
        void Header_WriteHeader(
            SHA256& Header);

        /// <summary>
        /// Write sections to the header.
        /// </summary>
        void Header_WriteSections(
            SHA256& Header);

        /// <summary>
        /// Write body to the header.
        /// </summary>
        void Header_WriteBody();

    private:
        std::fstream    m_FileStream;
        std::mutex      m_PackMutex;
        LoadedAssetsMap m_LoadedAssets;
        AssetInfoMap    m_AssetsInfo;
    };
} // namespace Neon::Asset