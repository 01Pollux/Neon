#include <EnginePCH.hpp>
#include <Asset/Handlers/Model.hpp>

#ifndef NEON_DIST
#include <AssImp/Importer.hpp>
#endif

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
        // Get extension from path.
        size_t ExtensionIndex = Path.find_last_of('.');
        if (ExtensionIndex == StringU8::npos)
        {
            return nullptr;
        }

        switch (StringUtils::Hash(Path.substr(ExtensionIndex + 1)))
        {
#ifndef NEON_DIST
        case StringUtils::Hash("obj"):
        case StringUtils::Hash("OBJ"):
        {
            Assimp::Importer Importer;
            Importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, true);
            break;
        }
#endif
        default:
            break;
        }
        auto Extension = Path.substr(ExtensionIndex + 1);

        auto Model = std::make_shared<Renderer::Model>();
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