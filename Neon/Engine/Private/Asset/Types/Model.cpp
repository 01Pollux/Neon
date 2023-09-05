#include <EnginePCH.hpp>
#include <Asset/Handlers/Model.hpp>

namespace Neon::Asset
{
    bool ModelAsset::Handler::CanHandle(
        const Ptr<IAsset>& Asset)
    {
        return dynamic_cast<ModelAsset*>(Asset.get());
    }

    Ptr<IAsset> ModelAsset::Handler::Load(
        std::istream& Stream,
        const Asset::DependencyReader&,
        const Handle&        AssetGuid,
        StringU8             Path,
        const AssetMetaData& LoaderData)
    {
        Ptr<Renderer::Model> Model;
        auto Asset = std::make_shared<ModelAsset>(Model, AssetGuid, std::move(Path));
        return Asset;
    }

    void ModelAsset::Handler::Save(
        std::iostream& Stream,
        DependencyWriter&,
        const Ptr<IAsset>& Asset,
        AssetMetaData&     LoaderData)
    {
        auto AssetPtr = static_cast<Asset::ModelAsset*>(Asset.get());
    }
} // namespace Neon::Asset