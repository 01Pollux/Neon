#pragma once

#include <Asset/Handler.hpp>
#include <Asset/Types/Texture.hpp>

namespace Neon::Asset
{
    class TextureAsset::Handler : public IAssetHandler
    {
        friend class Handler;

    public:
        bool CanHandle(
            const Ptr<IAsset>& Resource) override;

        Ptr<IAsset> Load(
            std::istream& Stream,
            const Asset::DependencyReader&,
            const Handle&        AssetGuid,
            StringU8             Path,
            const AssetMetaData& LoaderData) override;

        void Save(
            std::iostream& Stream,
            DependencyWriter&,
            const Ptr<IAsset>& Asset,
            AssetMetaData&     LoaderData) override;
    };
} // namespace Neon::Asset