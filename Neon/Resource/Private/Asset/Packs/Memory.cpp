#include <ResourcePCH.hpp>
#include <Asset/Packs/Memory.hpp>

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
        const Asset::Handle& AssetGuid)
    {
        RLock Lock(m_CacheMutex);
        return m_Cache.contains(AssetGuid);
    }
} // namespace Neon::Asset
