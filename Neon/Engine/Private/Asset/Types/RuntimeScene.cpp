#include <EnginePCH.hpp>
#include <Asset/Handlers/RuntimeScene.hpp>
#include <Scene/Component/Component.hpp>

#include <boost/serialization/map.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <Log/Logger.hpp>

namespace Neon::Asset
{
    /// <summary>
    /// Load an entity and its children.
    /// </summary>
    static void LoadEntityFromArchiveImpl(
        Scene::EntityHandle            SceneRoot,
        boost::archive::text_iarchive& Archive,
        Scene::EntityHandle            EntHandle)
    {
        flecs::entity Entity = EntHandle;

        StringU8 Name;
        Archive >> Name;
        Entity.set_name(Name.c_str());

        StringU8 TypeName;
        do
        {
            Archive >> TypeName;
            if (TypeName.empty())
            {
                break;
            }

            auto Component = Scene::EntityWorld::Get().lookup(TypeName.c_str());
            if (!Component)
            {
                NEON_ERROR("Missing entity component with name: {}", TypeName);
                continue;
            }

            auto Serializer = Component.get<Scene::Component::ComponentSerializer>();
            if (!Serializer)
            {
                NEON_ERROR("Missing entity component serializer with name: {}", TypeName);
                continue;
            }

            Entity.add(Component);
            Serializer->Deserialize(Archive, Entity, Component);
        } while (true);

        uint32_t Count;
        Archive >> Count;

        for (uint32_t i = 0; i < Count; i++)
        {
            flecs::entity Child = Scene::EntityHandle::Create(SceneRoot, EntHandle);
            LoadEntityFromArchiveImpl(SceneRoot, Archive, Child);
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

        auto SceneRoot = Asset->GetScene().GetRoot();

        for (uint32_t i = 0; i < Count; i++)
        {
            flecs::entity Child = Scene::EntityHandle::Create(SceneRoot);
            LoadEntityFromArchiveImpl(SceneRoot, Archive, Child);
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
        Archive << StringU8(Entity.name());

        Entity.each(
            [&Archive, &Entity](flecs::id ComponentId)
            {
                auto Component = Scene::EntityWorld::Get().component(ComponentId);
                if (!Component)
                {
                    return;
                }

                auto Serializer = Component.get<Scene::Component::ComponentSerializer>();
                if (!Serializer)
                {
                    return;
                }

                Archive << StringU8(ComponentId.str());
                Serializer->Serialize(Archive, Entity, ComponentId);
            });

        Archive << StringU8();

        auto ChildrenFilter = Scene::EntityWorld::GetChildrenFilter(Entity).build();
        Archive << uint32_t(ChildrenFilter.count());

        ChildrenFilter.each(
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
        auto RootFilter = Scene::EntityWorld::GetChildrenFilter(Asset->GetScene().GetRoot()).build();

        Archive << uint32_t(RootFilter.count());

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