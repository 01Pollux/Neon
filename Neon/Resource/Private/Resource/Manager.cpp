#include <ResourcePCH.hpp>
#include <Resource/Manager.hpp>

#include <Log/Logger.hpp>

namespace Neon::Asset
{
    void IResourceManager::AddHandler(
        UPtr<IAssetResourceHandler> Handler)
    {
        m_Handlers.Append(std::move(Handler));
    }

    //

    void IResourceManager::LoadPack(
        const StringU8& Path,
        const StringU8& Tag)
    {
        NEON_ASSERT(!m_LoadedPacks.contains(Tag), "Loading pack with duplicate tag");

        Ptr<IAssetPack> Pack = OpenPack(Path);
        NEON_VALIDATE(Pack);

        NEON_INFO("Resource", "Loading pack: '{:X}' with tag: '{}'", static_cast<void*>(Pack.get()), Tag);
        Pack->Import(Path);

        m_LoadedPacks.emplace(Tag, Pack);
    }

    void IResourceManager::UnloadPack(
        const StringU8& Tag)
    {
        bool Erased = m_LoadedPacks.erase(Tag) > 0;
        NEON_VALIDATE(Erased, "Resource pack '{}' doesn't exists", Tag);
    }

    //

    void IResourceManager::ImportPack(
        const StringU8& Tag,
        const StringU8& FilePath)
    {
        GetPack(Tag)->Import(FilePath);
    }

    void IResourceManager::ExportPack(
        const StringU8& Tag,
        const StringU8& FilePath)
    {
        GetPack(Tag)->Export(m_Handlers, FilePath);
    }

    Ref<IAssetResource> IResourceManager::LoadPack(
        const StringU8&    Tag,
        const AssetHandle& Handle)
    {
        return GetPack(Tag)->Load(m_Handlers, Handle);
    }

    void IResourceManager::SavePack(
        const StringU8&            Tag,
        const AssetHandle&         Handle,
        const Ptr<IAssetResource>& Resource)
    {
        GetPack(Tag)->Save(m_Handlers, Handle, Resource);
    }

    //

    IAssetPack* IResourceManager::GetPack(
        const StringU8& Tag)
    {
        auto Iter = m_LoadedPacks.find(Tag);
        NEON_VALIDATE(Iter == m_LoadedPacks.end(), "Resource pack '{}' doesn't exists", Tag);
        return Iter->second.get();
    }
} // namespace Neon::Asset