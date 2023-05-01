#pragma once

#include <Resource/Pack.hpp>
#include <Core/String.hpp>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/functional/hash.hpp>

namespace Neon::Asset
{
    class AssetPack : public IAssetPack
    {
        struct PackInfo
        {
            /// <summary>
            /// Hash of the data in the mapped filed
            /// </summary>
            // char Hash[41];

            /// <summary>
            /// Id of the loader to use
            /// </summary>
            uint32_t LoaderId;

            /// <summary>
            /// Offset of data in mapped file
            /// </summary>
            size_t Offset;

            /// <summary>
            /// Size of data in mapped file
            /// </summary>
            size_t Size;
        };

        using LoadedAssetsMap = std::unordered_map<boost::uuids::uuid, Ptr<IAssetResource>, boost::hash<boost::uuids::uuid>>;
        using PendingAssetMap = std::unordered_map<boost::uuids::uuid, PackInfo, boost::hash<boost::uuids::uuid>>;

        struct PendingPacksInfo
        {
            PendingAssetMap               PendingAssets;
            boost::iostreams::mapped_file FileView;
        };

    public:
        Ref<IAssetResource> Load(
            const AssetResourceHandlerMap& Handlers,
            const AssetHandle&             Handle) override;

        void UnrefAsset(
            const AssetHandle& Handle) override;

    private:
        Ref<IAssetResource> LoadAsset(
            const AssetResourceHandlerMap& Handlers,
            const AssetHandle&             Handle,
            StringU8&                      ErrorText);

    private:
        LoadedAssetsMap m_LoadedAssets;

        std::unique_ptr<PendingPacksInfo> m_PendingAssets;
    };
} // namespace Neon::Asset