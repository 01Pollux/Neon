#include <EnginePCH.hpp>
#include <Asset/Handlers/Json.hpp>

namespace Neon::Asset
{
    void JsonAsset::SetJson(
        boost::json::value Json)
    {
        MarkDirty();
        m_Json = std::move(Json);
    }

    //

    bool JsonAsset::Handler::CanHandle(
        const Ptr<IAsset>& Asset)
    {
        return dynamic_cast<JsonAsset*>(Asset.get());
    }

    Ptr<IAsset> JsonAsset::Handler::Load(
        std::istream& Stream,
        const Asset::DependencyReader&,
        const Handle&        AssetGuid,
        StringU8             Path,
        const AssetMetaData& LoaderData)
    {
        auto Asset = std::make_shared<JsonAsset>(AssetGuid, std::move(Path));

        boost::json::parse_options Options{
            .allow_comments = true
        };
        Asset->m_Json = boost::json::parse(Stream, {}, Options);

        return Asset;
    }

    void JsonAsset::Handler::Save(
        std::iostream& Stream,
        DependencyWriter&,
        const Ptr<IAsset>& Asset,
        AssetMetaData&     LoaderData)
    {
        auto AssetPtr = static_cast<Asset::JsonAsset*>(Asset.get());

        boost::json::string_view JsonString = boost::json::serialize(AssetPtr->GetJson());
        Stream.write(JsonString.data(), JsonString.size());
        Stream << '\0';
    }
} // namespace Neon::Asset