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
        NEON_ASSERT(!m_LoadedPacksTags.contains(Tag), "Loading pack with duplicate tag");

        uint16_t PackId = 0;
        for (auto& Id : m_LoadedPacks | std::views::keys)
        {
            if (PackId != Id)
            {
                break;
            }
            ++PackId;
        }
        NEON_ASSERT(!m_LoadedPacks.contains(PackId));

        Ptr<IAssetPack> Pack = OpenPack(Path, PackId);
        NEON_VALIDATE(Pack);

        NEON_INFO("Resource", "Loading pack: '{:X}' with tag: '{}' and id: '{}'", static_cast<void*>(Pack.get()), Tag, PackId);
        Pack->Import(Path);

        auto Iter = m_LoadedPacks.emplace(PackId, Pack).first;
        m_LoadedPacksTags.emplace(Tag, Iter);
    }

    void IResourceManager::UnloadPack(
        const StringU8& Tag)
    {
        auto Iter = m_LoadedPacksTags.find(Tag);
        NEON_ASSERT(Iter != m_LoadedPacksTags.end(), "Pack doesn't exists");

        m_LoadedPacks.erase(Iter->second);
        m_LoadedPacksTags.erase(Iter);
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
        auto Iter = m_LoadedPacksTags.find(Tag);
        NEON_VALIDATE(Iter == m_LoadedPacksTags.end(), "Resource pack '{}' doesn't exists", Tag);
        return Iter->second->second.get();
    }
} // namespace Neon::Asset