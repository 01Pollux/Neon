#pragma once

#include <Asset/Asset.hpp>
#include <Asset/Metadata.hpp>
#include <IO/Archive2.hpp>
#include <fstream>

namespace Neon::AAsset
{
    class IAssetHandler
    {
    public:
        /// <summary>
        /// Query if this asset handler can handle the given asset.
        /// </summary>
        virtual bool CanHandle(
            const Ptr<IAsset>& Asset) = 0;

        /// <summary>
        /// Load the asset from an input stream.
        /// </summary>
        virtual Ptr<IAsset> Load(
            std::ifstream&       Stream,
            const Handle&        AssetGuid,
            const AssetMetaData& LoaderData) = 0;

        /// <summary>
        /// Save the asset to an output stream.
        /// </summary>
        virtual void Save(
            std::fstream&      Stream,
            const Ptr<IAsset>& Asset,
            AssetMetaData&     LoaderData) = 0;
    };
} // namespace Neon::AAsset