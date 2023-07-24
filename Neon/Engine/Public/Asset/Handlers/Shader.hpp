#pragma once

#include <Asset/Handler.hpp>
#include <Asset/Types/Shader.hpp>

namespace Neon::Asset
{
    class ShaderAsset::Handler : public IAssetHandler
    {
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