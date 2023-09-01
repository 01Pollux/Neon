#include <EnginePCH.hpp>
#include <Asset/Handlers/RuntimeScene.hpp>

#include <boost/serialization/map.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

namespace Neon::Asset
{
    /// <summary>
    /// Create a filter to get all children of an entity regardless of whether they are disabled or not.
    /// </summary>
    [[nodiscard]] static flecs::filter<> GetChildrenFilter(
        const flecs::entity& Parent)
    {
        return Parent.world()
            .filter_builder()
            .term(flecs::ChildOf, Parent)
            .term(flecs::Disabled)
            .optional()
            .build();
    }

    //

    bool RuntimeSceneAsset::Handler::CanHandle(
        const Ptr<IAsset>& Asset)
    {
        return dynamic_cast<RuntimeSceneAsset*>(Asset.get());
    }

    Ptr<IAsset> RuntimeSceneAsset::Handler::Load(
        std::istream& Stream,
        const Asset::DependencyReader&,
        const Handle&        AssetGuid,
        StringU8             Path,
        const AssetMetaData& LoaderData)
    {
        boost::archive::text_iarchive Archive(Stream);

        StringU8 Json;
        Archive >> Json;

        flecs::world World;
        World.from_json(Json.c_str());

        return std::make_shared<RuntimeSceneAsset>(AssetGuid, std::move(Path));
    }

    void RuntimeSceneAsset::Handler::Save(
        std::iostream& Stream,
        DependencyWriter&,
        const Ptr<IAsset>& Asset,
        AssetMetaData&     LoaderData)
    {
        auto RuntimeSceneAsset = static_cast<Asset::RuntimeSceneAsset*>(Asset.get());

        // auto World = RuntimeSceneAsset->GetScene().GetWorld();

        // boost::archive::text_oarchive Archive(Stream);

        // auto Json = World.to_json();
        // Archive << StringU8(Json.c_str(), Json.size());
    }
} // namespace Neon::Asset