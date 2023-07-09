#include <ResourcePCH.hpp>
#include <Resource/Manager.hpp>
#include <Resource/Pack.hpp>

#include <Log/Logger.hpp>

namespace Neon::Asset
{
    void IAssetManager::AddHandler(
        UPtr<IAssetResourceHandler> Handler)
    {
        m_Handlers.Append(std::move(Handler));
    }

    //

    auto IAssetManager::GetPacks() const -> const AssetPackMap&
    {
        return m_LoadedPacks;
    }

    const Ptr<IAssetPack>& IAssetManager::GetPack(
        const StringU8& Tag)
    {
        auto Iter = m_LoadedPacks.find(Tag);
        NEON_VALIDATE(Iter != m_LoadedPacks.end(), "Resource pack '{}' doesn't exists", Tag);
        return Iter->second;
    }

    Ptr<IAssetPack> IAssetManager::TryLoadPack(
        const StringU8& Tag,
        const StringU8& Path)
    {
        PackIsUnique(Tag);

        auto Pack = OpenPack(Path);
        if (Pack)
        {
            NEON_TRACE_TAG("Resource", "Loading pack: '{}' with tag: '{}'", static_cast<void*>(Pack.get()), Tag);
            Pack->ImportAsync(Path);

            return m_LoadedPacks.emplace(Tag, std::move(Pack)).first->second;
        }
        else
        {
            return nullptr;
        }
    }

    Ptr<IAssetPack> IAssetManager::LoadPack(
        const StringU8& Tag,
        const StringU8& Path)
    {
        auto Pack = TryLoadPack(Tag, Path);
        NEON_VALIDATE(Pack);
        return Pack;
    }

    void IAssetManager::UnloadPack(
        const StringU8& Tag)
    {
        bool Erased = m_LoadedPacks.erase(Tag) > 0;
        NEON_VALIDATE(Erased, "Resource pack '{}' doesn't exists", Tag);
    }

    void IAssetManager::PackIsUnique(
        const StringU8& Tag) const
    {
        NEON_ASSERT(!m_LoadedPacks.contains(Tag), "Loading pack with duplicate tag");
    }
} // namespace Neon::Asset