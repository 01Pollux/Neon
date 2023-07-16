#include <ResourcePCH.hpp>
#include <Asset/Storage.hpp>
#include <Asset/Package.hpp>
#include <Asset/Handler.hpp>

#include <Log/Logger.hpp>

namespace Neon::AAsset
{
    Storage::~Storage()
    {
        printf("dtor\n");
    }

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

    std::future<Ref<IAsset>> Storage::Load(
        const Handle& ResHandle)
    {
        auto Task = [this, ResHandle]() -> Ref<IAsset>
        {
            {
                std::scoped_lock CacheLock(m_AssetCacheMutex);
                if (auto It = m_AssetCache.find(ResHandle); It != m_AssetCache.end())
                {
                    return It->second;
                }
            }

            Ptr<IAsset> Asset;
            {
                std::scoped_lock PackageLock(m_AssetsInPackageMutex);
                if (auto It = m_AssetsInPackage.find(ResHandle); It != m_AssetsInPackage.end())
                {
                    Asset = It->second->Load(this, ResHandle);
                }
            }

            if (Asset)
            {
                std::scoped_lock CacheLock(m_AssetCacheMutex);
                m_AssetCache.emplace(ResHandle, Asset);
            }
            else
            {
                NEON_ERROR_TAG("Asset", "Asset '{}' does not exist", ResHandle.ToString());
            }

            return Asset;
        };

        return m_ThreadPool.enqueue(std::move(Task));
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