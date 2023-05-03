#pragma once

#include <Core/Neon.hpp>
#include <Core/String.hpp>

#include <Resource/Asset.hpp>

namespace Neon::Asset
{
    class AssetResourceHandlers;
    class IAssetPack
    {
    public:
        IAssetPack(
            uint16_t PackId) :
            m_PackId(PackId)
        {
        }

        virtual ~IAssetPack() = default;

        /// <summary>
        /// Import asset pack file and overwrite current content of the pack.
        /// </summary>
        virtual void Import(
            const StringU8& FilePath) = 0;

        /// <summary>
        /// Export asset pack file.
        /// </summary>
        virtual void Export(
            const AssetResourceHandlers& Handlers,
            const StringU8&              FilePath) = 0;

        /// <summary>
        /// Load asset from pack file.
        /// </summary>
        virtual Ref<IAssetResource> Load(
            const AssetResourceHandlers& Handlers,
            const AssetHandle&           Handle) = 0;

        /// <summary>
        /// Save asset to pack file.
        /// </summary>
        virtual void Save(
            const AssetResourceHandlers& Handlers,
            const AssetHandle&           Handle,
            const Ptr<IAssetResource>&   Resource) = 0;

    protected:
        uint16_t m_PackId;
    };
} // namespace Neon::Asset