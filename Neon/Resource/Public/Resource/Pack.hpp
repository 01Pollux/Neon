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
            const AssetResourceHandlers& Handlers) noexcept :
            m_Handlers(Handlers)
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
            const StringU8& FilePath) = 0;

        /// <summary>
        /// Load asset from pack file.
        /// </summary>
        virtual Ref<IAssetResource> Load(
            const AssetHandle& Handle) = 0;

        /// <summary>
        /// Save asset to pack file.
        /// </summary>
        virtual void Save(
            const AssetHandle&         Handle,
            const Ptr<IAssetResource>& Resource) = 0;

    protected:
        const AssetResourceHandlers& m_Handlers;
    };
} // namespace Neon::Asset