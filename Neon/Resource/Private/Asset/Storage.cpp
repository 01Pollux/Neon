#include <ResourcePCH.hpp>
#include <Private/Asset/Storage.hpp>

#include <Asset/Pack.hpp>
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

    void Storage::AddAsset(
        const AddDesc& Desc)
    {
        s_Instance->AddAsset(Desc);
    }

    void Storage::RemoveAsset(
        const Handle& AssetGuid)
    {
        s_Instance->RemoveAsset(AssetGuid);
    }

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

    StorageImpl::StorageImpl()
    {
        Mount(UPtr<IAssetPackage>(NEON_NEW MemoryAssetPackage));
    }

    void StorageImpl::AddAsset(
        const AddDesc& Desc)
    {
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

        Package->AddAsset(Desc.Asset);
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

        NEON_WARNING_TAG("Asset", "Asset : '{}' not found", AssetGuid.ToString());
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