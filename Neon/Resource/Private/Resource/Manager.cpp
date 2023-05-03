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

    IAssetPack* IResourceManager::GetPack(
        const StringU8& Tag)
    {
        auto Iter = m_LoadedPacks.find(Tag);
        NEON_VALIDATE(Iter == m_LoadedPacks.end(), "Resource pack '{}' doesn't exists", Tag);
        return Iter->second.get();
    }

    IAssetPack* IResourceManager::LoadPack(
        const StringU8& Tag,
        const StringU8& Path)
    {
        PackIsUnique(Tag);

        UPtr<IAssetPack> Pack = OpenPack(Path);
        NEON_VALIDATE(Pack);

        NEON_INFO("Resource", "Loading pack: '{:X}' with tag: '{}'", static_cast<void*>(Pack.get()), Tag);
        Pack->Import(Path);

        return m_LoadedPacks.emplace(Tag, std::move(Pack)).first->second.get();
    }

    void IResourceManager::UnloadPack(
        const StringU8& Tag)
    {
        bool Erased = m_LoadedPacks.erase(Tag) > 0;
        NEON_VALIDATE(Erased, "Resource pack '{}' doesn't exists", Tag);
    }

    void IResourceManager::PackIsUnique(
        const StringU8& Tag) const
    {
        NEON_ASSERT(!m_LoadedPacks.contains(Tag), "Loading pack with duplicate tag");
    }
} // namespace Neon::Asset