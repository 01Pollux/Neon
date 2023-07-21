#include <ResourcePCH.hpp>
#include <Private/Asset/Storage.hpp>
#include <Private/Asset/Manager.hpp>

#include <Asset/Pack.hpp>
#include <Asset/Handler.hpp>
#include <Asset/Packs/Memory.hpp>

#include <Log/Logger.hpp>

namespace Neon::AAsset
{
    static inline UPtr<StorageImpl> s_Instance;

    //

    void Storage::Initialize()
    {
        NEON_ASSERT(!s_Instance, "Storage already initialized");
        s_Instance = std::make_unique<StorageImpl>();
    }

    void Storage::Shutdown()
    {
        NEON_ASSERT(s_Instance, "Storage not initialized");
        s_Instance.reset();
    }

    StorageImpl* StorageImpl::Get()
    {
        return s_Instance.get();
    }

    //

    std::future<void> Storage::AddAsset(
        const AddDesc& Desc)
    {
        return s_Instance->AddAsset(Desc);
    }

    void Storage::RemoveAsset(
        const Handle& AssetGuid)
    {
        s_Instance->RemoveAsset(AssetGuid);
    }

    //

    IAssetHandler* Storage::RegisterHandler(
        UPtr<IAssetHandler> Handler)
    {
        return s_Instance->RegisterHandler(std::move(Handler));
    }

    void Storage::UnregisterHandler(
        IAssetHandler* Handler)
    {
        s_Instance->UnregisterHandler(Handler);
    }

    IAssetHandler* Storage::GetHandler(
        const Ptr<IAsset>& Asset)
    {
        return s_Instance->GetHandler(Asset);
    }

    //

    IAssetPackage* Storage::Mount(
        UPtr<IAssetPackage> Package)
    {
        return s_Instance->Mount(std::move(Package));
    }

    void Storage::Unmount(
        IAssetPackage* Package)
    {
        s_Instance->Unmount(Package);
    }

    Asio::CoGenerator<IAssetPackage*> Storage::GetPackages(
        bool IncludeMemoryOnly)
    {
        return s_Instance->GetPackages(IncludeMemoryOnly);
    }

    Asio::CoGenerator<Storage::PackageAndAsset> Storage::GetAllAssets(
        bool IncludeMemoryOnly)
    {
        return s_Instance->GetAllAssets(IncludeMemoryOnly);
    }

    //

    ManagerImpl* StorageImpl::GetManager()
    {
        return &m_Manager;
    }

    StorageImpl::StorageImpl()
    {
        Mount(UPtr<IAssetPackage>(NEON_NEW MemoryAssetPackage));
    }

    Asio::ThreadPool<>& StorageImpl::GetThreadPool()
    {
        return m_ThreadPool;
    }

    std::future<void> StorageImpl::AddAsset(
        const AddDesc& Desc)
    {
        NEON_ASSERT(Desc.Asset, "Asset is null");
        NEON_ASSERT(!Desc.Path.empty(), "Asset Path is empty");

        IAssetPackage* Package = Desc.PreferredPackage;
        if (Desc.MemoryOnly)
        {
            Package = m_Packages.front().get();
        }
        else if (!Package)
        {
            NEON_ASSERT(m_Packages.size() > 1, "No packages mounted");
            Package = std::next(m_Packages.begin())->get();
        }
#if NEON_DEBUG
        else
        {
            if (std::ranges::find_if(
                    m_Packages, [Package](const auto& CurPackage)
                    { return CurPackage.get() == Package; }) == m_Packages.end())
            {
                NEON_ASSERT(false, "Package not mounted");
            }
        }
#endif

        return Package->AddAsset(Desc.Asset, Desc.Path);
    }

    void StorageImpl::RemoveAsset(
        const Handle& AssetGuid)
    {
        for (auto& Package : m_Packages)
        {
            if (Package->RemoveAsset(AssetGuid))
            {
                return;
            }
        }

        NEON_WARNING_TAG("Asset", "Asset '{}' not found", AssetGuid.ToString());
    }

    IAssetHandler* StorageImpl::RegisterHandler(
        UPtr<IAssetHandler> Handler)
    {
        return m_Handlers.emplace_back(std::move(Handler)).get();
    }

    void StorageImpl::UnregisterHandler(
        IAssetHandler* Handler)
    {
        std::erase_if(
            m_Handlers, [Handler](const auto& CurHandler)
            { return CurHandler.get() == Handler; });
    }

    IAssetHandler* StorageImpl::GetHandler(
        const Ptr<IAsset>& Asset)
    {
        for (auto& Handler : m_Handlers)
        {
            if (Handler->CanHandle(Asset))
            {
                return Handler.get();
            }
        }

        return nullptr;
    }

    IAssetPackage* StorageImpl::Mount(
        UPtr<IAssetPackage> Package)
    {
        return m_Packages.emplace_back(std::move(Package)).get();
    }

    void StorageImpl::Unmount(
        IAssetPackage* Package)
    {
        std::erase_if(
            m_Packages, [Package](const auto& CurPackage)
            { return CurPackage.get() == Package; });
    }

    Asio::CoGenerator<IAssetPackage*> StorageImpl::GetPackages(
        bool IncludeMemoryOnly)
    {
        auto Iter = m_Packages.begin();
        if (!IncludeMemoryOnly)
        {
            ++Iter;
        }
        for (; Iter != m_Packages.end(); ++Iter)
        {
            co_yield Iter->get();
        }
    }

    Asio::CoGenerator<Storage::PackageAndAsset> StorageImpl::GetAllAssets(
        bool IncludeMemoryOnly)
    {
        auto Iter = m_Packages.begin();
        if (!IncludeMemoryOnly)
        {
            ++Iter;
        }
        for (; Iter != m_Packages.end(); ++Iter)
        {
            for (auto& AssetGuid : (*Iter)->GetAssets())
            {
                co_yield { Iter->get(), AssetGuid };
            }
        }
    }
} // namespace Neon::AAsset