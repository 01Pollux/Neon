#include <ResourcePCH.hpp>
#include <Private/Asset/Storage.hpp>

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
        std::vector<std::future<void>> Futures;

        for (auto& Package : StorageImpl::Get()->GetPackages(false))
        {
            Futures.emplace_back(Package->Export());
        }

        try
        {
            for (auto& Future : Futures)
            {
                Future.get();
            }
        }
        catch (const std::exception& Exception)
        {
            NEON_ERROR_TAG("Asset", "Failed to export packages: {}", Exception.what());
        }

        NEON_ASSERT(s_Instance, "Storage not initialized");
        s_Instance.reset();
    }

    StorageImpl* StorageImpl::Get()
    {
        return s_Instance.get();
    }

    //

    std::future<void> Storage::SaveAsset(
        const AddDesc& Desc)
    {
        return s_Instance->SaveAsset(Desc);
    }

    void Storage::RemoveAsset(
        const Handle& AssetGuid)
    {
        s_Instance->RemoveAsset(AssetGuid);
    }

    //

    void Storage::RegisterHandler(
        size_t              Id,
        UPtr<IAssetHandler> Handler)
    {
        s_Instance->RegisterHandler(Id, std::move(Handler));
    }

    void Storage::UnregisterHandler(
        size_t Id)
    {
        s_Instance->UnregisterHandler(Id);
    }

    IAssetHandler* Storage::GetHandler(
        const Ptr<IAsset>& Asset,
        size_t*            Id)
    {
        return s_Instance->GetHandler(Asset, Id);
    }

    IAssetHandler* Storage::GetHandler(
        size_t Id)
    {
        return s_Instance->GetHandler(Id);
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

    StorageImpl::~StorageImpl()
    {
    }

    Asio::ThreadPool<>& StorageImpl::GetThreadPool()
    {
        return m_ThreadPool;
    }

    std::future<void> StorageImpl::SaveAsset(
        const AddDesc& Desc)
    {
        NEON_ASSERT(Desc.Asset, "Asset is null");

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

        return Package->SaveAsset(Desc.Asset);
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

    void StorageImpl::RegisterHandler(
        size_t              Id,
        UPtr<IAssetHandler> Handler)
    {
        NEON_ASSERT(Id, "Invalid handler id");
        if (!m_Handlers.emplace(Id, std::move(Handler)).second)
        {
            NEON_ASSERT(false, "Handler already registered");
        }
    }

    void StorageImpl::UnregisterHandler(
        size_t Id)
    {
        NEON_ASSERT(Id, "Invalid handler id");
        m_Handlers.erase(Id);
    }

    IAssetHandler* StorageImpl::GetHandler(
        const Ptr<IAsset>& Asset,
        size_t*            Id)
    {
        for (auto& [HandlerId, Handler] : m_Handlers)
        {
            if (Handler->CanHandle(Asset))
            {
                if (HandlerId)
                {
                    *Id = HandlerId;
                }
                return Handler.get();
            }
        }

        return nullptr;
    }

    IAssetHandler* StorageImpl::GetHandler(
        size_t Id)
    {
        NEON_ASSERT(Id, "Invalid handler id");

        auto Iter = m_Handlers.find(Id);
        return Iter != m_Handlers.end() ? Iter->second.get() : nullptr;
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