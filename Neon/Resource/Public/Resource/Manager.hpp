#pragma once

#include <Core/Neon.hpp>
#include <Core/String.hpp>
#include <Resource/Pack.hpp>

namespace Neon::Asset
{
    class IResourceManager
    {
        using AssetPackMap            = std::map<uint16_t, Ptr<IAssetPack>>;
        using AssetResourceHandlerMap = std::map<uint32_t, UPtr<IAssetResourceHandler>>;

    public:
        /// <summary>
        /// Import pak file
        /// </summary>
        void Import(
            const StringU8& Path);

        /// <summary>
        /// Load asset from pack file.
        /// </summary>
        Ref<IAssetResource> Load(
            const AssetHandle& Handle);

        /// <summary>
        /// Save asset to pack file.
        /// </summary>
        void Save(
            const AssetHandle&         Handle,
            const Ptr<IAssetResource>& Resource);

    private:
        AssetResourceHandlerMap m_Handlers;
        AssetPackMap            m_LoadedPacks;
    };
} // namespace Neon::Asset