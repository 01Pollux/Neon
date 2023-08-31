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
        auto Asset = std::make_shared<RuntimeSceneAsset>(AssetGuid, std::move(Path));
        auto Root  = Asset->GetScene().GetRoot().Get();

        boost::archive::text_iarchive Archive(Stream);

        int32_t Size;
        Archive >> Size;
        std::string JsonString;
        for (int32_t i = 0; i < Size; ++i)
        {
            Archive >> JsonString;
            flecs::entity Entity;
            Entity.from_json(JsonString.c_str());
            Entity.child_of(Root);
        }

        return Asset;
    }

    void RuntimeSceneAsset::Handler::Save(
        std::iostream& Stream,
        DependencyWriter&,
        const Ptr<IAsset>& Asset,
        AssetMetaData&     LoaderData)
    {
        auto RuntimeSceneAsset = static_cast<Asset::RuntimeSceneAsset*>(Asset.get());

        auto Root = RuntimeSceneAsset->GetScene().GetRoot().Get();

        boost::archive::text_oarchive Archive(Stream);

        auto Filter = GetChildrenFilter(Root);
        Archive << int32_t(Filter.count());

        std::string JsonString;
        Filter.each(
            [&Archive, &JsonString](flecs::entity Entity)
            {
                JsonString = ecs_entity_to_json(Entity.world(), Entity, nullptr);
                Archive << JsonString;
            });
    }
} // namespace Neon::Asset