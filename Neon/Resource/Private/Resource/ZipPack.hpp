#pragma once

#include <Resource/Pack.hpp>
#include <Core/String.hpp>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/functional/hash.hpp>

namespace Neon::Asset
{
    class ZipAssetPack : public IAssetPack
    {
        struct PackInfo
        {
            /// <summary>
            /// Hash of the data in the mapped filed
            /// </summary>
            std::array<uint8_t, 32> Hash;

            /// <summary>
            /// Id of the loader to use
            /// </summary>
            uint32_t LoaderId = 0;

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
            const StringU8& FilePath) override;

        Ref<IAssetResource> Load(
            const AssetResourceHandlerMap& Handlers,
            const AssetHandle&             Handle) override;

        void Save(
            const AssetResourceHandlerMap& Handlers,
            const AssetHandle&             Handle,
            const Ptr<IAssetResource>&     Resource);

    private:
        Ref<IAssetResource> LoadAsset(
            const AssetResourceHandlerMap& Handlers,
            const AssetHandle&             Handle,
            StringU8&                      ErrorText);

    private:
        bool ReadHeader();

    private:
        boost::iostreams::mapped_file m_FileView;
        size_t                        m_DataSize = 0;

        LoadedAssetsMap m_LoadedAssets;
        AssetInfoMap    m_AssetsInfo;
    };
} // namespace Neon::Asset