#pragma once

#include <Asset/Handler.hpp>
#include <Asset/Types/TextFile.hpp>

namespace Neon::Asset
{
    class TextFileAsset::Handler : public IAssetHandler
    {
    public:
        bool CanHandle(
            const Ptr<IAsset>& Resource) override;

        Ptr<IAsset> Load(
            std::ifstream&                 Stream,
            const Asset::DependencyReader& DepReader,
            const Handle&                  AssetGuid,
            StringU8                       Path,
            const AssetMetaData&           LoaderData) override;

        void Save(
            std::fstream&      Stream,
            DependencyWriter&  DepWriter,
            const Ptr<IAsset>& Asset,
            AssetMetaData&     LoaderData) override;
    };
} // namespace Neon::Asset