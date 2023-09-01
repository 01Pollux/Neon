#include <EnginePCH.hpp>
#include <Asset/Handlers/RuntimeScene.hpp>
#include <Scene/Component/Component.hpp>

#include <boost/serialization/map.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

namespace Neon::Asset
{
    /// <summary>
    /// Load an entity and its children.
    /// </summary>
    static void LoadEntityFromArchiveImpl(
        Scene::EntityHandle            SceneTag,
        boost::archive::text_iarchive& Archive,
        Scene::EntityHandle            Entity)
    {
        StringU8 Json;
        Archive >> Json;

        Entity.Get().from_json(Json.c_str());

        uint32_t Count;
        Archive >> Count;

        for (uint32_t i = 0; i < Count; i++)
        {
            flecs::entity Child = Scene::EntityHandle::Create(SceneTag, Entity);
            LoadEntityFromArchiveImpl(SceneTag, Archive, Child);
        }
    }

    /// <summary>
    /// Load an entity and its children.
    /// </summary>
    static void LoadEntityFromArchive(
        boost::archive::text_iarchive& Archive,
        RuntimeSceneAsset*             Asset)
    {
        uint32_t Count;
        Archive >> Count;

        auto SceneTag = Asset->GetScene().GetTag();

        for (uint32_t i = 0; i < Count; i++)
        {
            flecs::entity Child = Scene::EntityHandle::Create(SceneTag);
            LoadEntityFromArchiveImpl(SceneTag, Archive, Child);
        }
    }

    //

    /// <summary>
    /// Save an entity and its children.
    /// </summary>
    static void SaveEntityToArchiveImpl(
        boost::archive::text_oarchive& Archive,
        flecs::entity                  Entity)
    {
        Archive << StringU8{ Entity.to_json() };

        auto ChidlrenFilter = Scene::EntityWorld::GetChildrenFilter(Entity).build();
        Archive << uint32_t(ChidlrenFilter.count());

        ChidlrenFilter.each(
            [&Archive](const flecs::entity& Child)
            {
                SaveEntityToArchiveImpl(Archive, Child);
            });
    }

    /// <summary>
    /// Save an entity and its children.
    /// </summary>
    static void SaveEntityToArchive(
        boost::archive::text_oarchive& Archive,
        RuntimeSceneAsset*             Asset)
    {
        auto SceneTag   = Asset->GetScene().GetTag();
        auto RootFilter = Scene::EntityWorld::GetRootFilter(SceneTag).build();

        uint32_t Count = RootFilter.count();
        Archive << Count;

        RootFilter.each(
            [&Archive](const flecs::entity& Entity)
            {
                SaveEntityToArchiveImpl(Archive, Entity);
            });
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

        boost::archive::text_iarchive Archive(Stream);
        LoadEntityFromArchive(Archive, Asset.get());

        return Asset;
    }

    void RuntimeSceneAsset::Handler::Save(
        std::iostream& Stream,
        DependencyWriter&,
        const Ptr<IAsset>& Asset,
        AssetMetaData&     LoaderData)
    {
        auto SceneAsset = static_cast<Asset::RuntimeSceneAsset*>(Asset.get());

        boost::archive::text_oarchive Archive(Stream);
        SaveEntityToArchive(Archive, SceneAsset);
    }
} // namespace Neon::Asset