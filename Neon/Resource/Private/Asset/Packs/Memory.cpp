#include <ResourcePCH.hpp>
#include <Asset/Packs/Memory.hpp>

namespace Neon::AAsset
{
    Asio::CoGenerator<const AAsset::Handle&> MemoryAssetPackage::GetAssets()
    {
        RLock Lock(m_CacheMutex);
        for (auto& Guid : m_Cache | std::views::keys)
        {
            co_yield Guid;
        }
    }

    bool MemoryAssetPackage::ContainsAsset(
        const AAsset::Handle& AssetGuid) const
    {
        RLock Lock(m_CacheMutex);
        return m_Cache.contains(AssetGuid);
    }

    std::future<void> MemoryAssetPackage::AddAsset(
        Ptr<IAsset> Asset,
        const StringU8&)
    {
        RWLock Lock(m_CacheMutex);
        m_Cache[Asset->GetGuid()] = std::move(Asset);
        return std::async(std::launch::deferred, [] {});
    }

    bool MemoryAssetPackage::RemoveAsset(
        const AAsset::Handle& AssetGuid)
    {
        RWLock Lock(m_CacheMutex);
        return m_Cache.erase(AssetGuid) > 0;
    }

    Ptr<IAsset> MemoryAssetPackage::LoadAsset(
        const AAsset::Handle& AssetGuid)
    {
        RWLock Lock(m_CacheMutex);
        auto   Iter = m_Cache.find(AssetGuid);
        return Iter != m_Cache.end() ? Iter->second : nullptr;
    }

    bool MemoryAssetPackage::UnloadAsset(
        const AAsset::Handle& AssetGuid)
    {
        RLock Lock(m_CacheMutex);
        return m_Cache.contains(AssetGuid);
    }
} // namespace Neon::AAsset
