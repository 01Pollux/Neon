#include <ResourcePCH.hpp>
#include <Asset/Storage.hpp>
#include <Asset/Package.hpp>
#include <Asset/Handler.hpp>

#include <cppcoro/schedule_on.hpp>

#include <Log/Logger.hpp>

namespace Neon::AAsset
{
    void Storage::RegisterHandler(
        const StringU8&     Name,
        UPtr<IAssetHandler> Handler)
    {
        auto& OldHandler = m_AssetHandlers[Name];
        if (OldHandler)
        {
            NEON_WARNING_TAG("Asset", "Trying to register an asset handler that already exists");
        }
        else
        {
            OldHandler = std::move(Handler);
        }
    }

    void Storage::UnregisterHandler(
        const StringU8& Name)
    {
        m_AssetHandlers.erase(Name);
    }

    IAssetHandler* Storage::GetHandler(
        const StringU8& Name)
    {
        auto Iter = m_AssetHandlers.find(Name);
        return Iter != m_AssetHandlers.end() ? Iter->second.get() : nullptr;
    }

    //

    void Storage::LoadAsync(
        const Handle& ResHandle)
    {
    }

    cppcoro::task<Ref<IAsset>> Storage::Load(
        const Handle& ResHandle)
    {
        {
            auto CacheLock = co_await m_AssetCacheMutex.scoped_lock_async();
            if (auto It = m_AssetCache.find(ResHandle); It != m_AssetCache.end())
            {
                co_return It->second;
            }
        }

        co_await m_ThreadPool.schedule();

        {
            auto PackageLock = co_await m_AssetsInPackageMutex.scoped_lock_async();
            if (auto It = m_AssetsInPackage.find(ResHandle); It != m_AssetsInPackage.end())
            {
                auto Asset = co_await It->second->Load(this, ResHandle);
                m_AssetCache.emplace(ResHandle, Asset);
                co_return Asset;
            }
        }

        NEON_ERROR_TAG("Asset", "Trying to load non-existing asset");
        co_return Ref<IAsset>();
    }

    void Storage::Unload(
        const Handle& ResHandle)
    {
    }

    //

    void Storage::Reference(
        IPackage*     Package,
        const Handle& ResHandle)
    {
        m_AssetsInPackage[ResHandle] = Package;
    }

    void Storage::Unreference(
        IPackage*     Package,
        const Handle& ResHandle)
    {
        m_AssetsInPackage.erase(ResHandle);
        m_AssetCache.erase(ResHandle);
    }
} // namespace Neon::AAsset