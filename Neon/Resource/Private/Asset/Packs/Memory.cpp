#include <ResourcePCH.hpp>
#include <Asset/Packs/Memory.hpp>
#include <regex>

namespace Neon::Asset
{
    Asio::CoGenerator<const Asset::Handle&> MemoryAssetPackage::GetAssets()
    {
        RLock Lock(m_CacheMutex);
        for (auto& Guid : m_Cache | std::views::keys)
        {
            co_yield Guid;
        }
    }

    bool MemoryAssetPackage::ContainsAsset(
        const Asset::Handle& AssetGuid) const
    {
        RLock Lock(m_CacheMutex);
        return m_Cache.contains(AssetGuid);
    }

    //

    Asset::Handle MemoryAssetPackage::FindAsset(
        const StringU8& Path) const
    {
        RLock Lock(m_CacheMutex);
        for (auto& [Guid, Asset] : m_Cache)
        {
            if (Asset->GetPath() == Path)
            {
                return Guid;
            }
        }
        return Asset::Handle::Null;
    }

    Asio::CoGenerator<Asset::Handle> MemoryAssetPackage::FindAssets(
        const StringU8& PathRegex) const
    {
        std::regex Regex(PathRegex);

        RLock Lock(m_CacheMutex);
        for (auto& [Guid, Asset] : m_Cache)
        {
            if (std::regex_match(Asset->GetPath(), Regex))
            {
                co_yield Asset::Handle{ Guid };
            }
        }
    }

    //

    std::future<void> MemoryAssetPackage::Export()
    {
        return std::async(std::launch::async, [] {});
    }

    std::future<void> MemoryAssetPackage::SaveAsset(
        Ptr<IAsset> Asset)
    {
        RWLock Lock(m_CacheMutex);
        m_Cache[Asset->GetGuid()] = std::move(Asset);
        return std::async(std::launch::deferred, [] {});
    }

    bool MemoryAssetPackage::RemoveAsset(
        const Asset::Handle& AssetGuid)
    {
        RWLock Lock(m_CacheMutex);
        return m_Cache.erase(AssetGuid) > 0;
    }

    Ptr<IAsset> MemoryAssetPackage::LoadAsset(
        const Asset::Handle& AssetGuid)
    {
        RWLock Lock(m_CacheMutex);
        auto   Iter = m_Cache.find(AssetGuid);
        return Iter != m_Cache.end() ? Iter->second : nullptr;
    }

    bool MemoryAssetPackage::UnloadAsset(
        const Asset::Handle& AssetGuid,
        bool                 Force)
    {
        RWLock Lock(m_CacheMutex);

        auto Iter = m_Cache.find(AssetGuid);
        if (Iter == m_Cache.end())
        {
            return false;
        }

        if (!Force && Iter->second.use_count() == 1)
        {
            m_Cache.erase(Iter);
        }
        return true;
    }
} // namespace Neon::Asset
