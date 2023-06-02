#include <ResourcePCH.hpp>
#include <Resource/Manager.hpp>
#include <Resource/Pack.hpp>

#include <Log/Logger.hpp>

namespace Neon::Asset
{
    void IResourceManager::AddHandler(
        UPtr<IAssetResourceHandler> Handler)
    {
        m_Handlers.Append(std::move(Handler));
    }

    //

    auto IResourceManager::GetPacks() const -> const AssetPackMap&
    {
        return m_LoadedPacks;
    }

    IAssetPack* IResourceManager::GetPack(
        const StringU8& Tag)
    {
        auto Iter = m_LoadedPacks.find(Tag);
        NEON_VALIDATE(Iter == m_LoadedPacks.end(), "Resource pack '{}' doesn't exists", Tag);
        return Iter->second.get();
    }

    IAssetPack* IResourceManager::TryLoadPack(
        const StringU8& Tag,
        const StringU8& Path)
    {
        PackIsUnique(Tag);

        UPtr<IAssetPack> Pack = OpenPack(Path);
        if (Pack)
        {
            NEON_INFO_TAG("Resource", "Loading pack: '{}' with tag: '{}'", static_cast<void*>(Pack.get()), Tag);
            Pack->ImportAsync(Path);

            return m_LoadedPacks.emplace(Tag, std::move(Pack)).first->second.get();
        }
        else
        {
            return nullptr;
        }
    }

    IAssetPack* IResourceManager::LoadPack(
        const StringU8& Tag,
        const StringU8& Path)
    {
        auto Pack = TryLoadPack(Tag, Path);
        NEON_VALIDATE(Pack);
        return Pack;
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