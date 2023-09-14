#include <ResourcePCH.hpp>
#include <Private/Asset/Manager.hpp>
#include <Private/Asset/Storage.hpp>
#include <Asset/Pack.hpp>

#include <Log/Logger.hpp>

namespace Neon::Asset
{
    ManagerImpl* ManagerImpl::Get()
    {
        return StorageImpl::Get()->GetManager();
    }

    //

    std::future<Ptr<IAsset>> Manager::Load(
        const Handle& AssetGuid)
    {
        return ManagerImpl::Get()->Load(AssetGuid);
    }

    std::future<Ptr<IAsset>> Manager::Reload(
        const Handle& AssetGuid)
    {
        return ManagerImpl::Get()->Reload(AssetGuid);
    }

    void Manager::Unload(
        const Handle& AssetGuid)
    {
        ManagerImpl::Get()->Unload(AssetGuid);
    }

    void Manager::RequestUnload(
        const Handle& AssetGuid)
    {
        ManagerImpl::Get()->RequestUnload(AssetGuid);
    }

    //

    std::future<Ptr<IAsset>> ManagerImpl::Load(
        const Handle& AssetGuid)
    {
        for (auto Package : Storage::GetPackages(true, true))
        {
            if (Package->ContainsAsset(AssetGuid))
            {
                return StorageImpl::Get()->GetThreadPool().enqueue(
                    [Package, AssetGuid]()
                    {
                        return Package->LoadAsset(AssetGuid);
                    });
            }
        }

        NEON_ERROR_TAG("Asset", "Asset not found: {}", AssetGuid.ToString());
        return {};
    }

    std::future<Ptr<IAsset>> ManagerImpl::Reload(
        const Handle& AssetGuid)
    {
        for (auto Package : Storage::GetPackages(true, true))
        {
            if (Package->RemoveAsset(AssetGuid, true))
            {
                break;
            }
        }
        return Load(AssetGuid);
    }

    bool ManagerImpl::Unload(
        const Handle& AssetGuid)
    {
        for (auto Package : Storage::GetPackages(true, true))
        {
            if (Package->RemoveAsset(AssetGuid, true))
            {
                return true;
            }
        }

        NEON_ERROR_TAG("Asset", "Asset not found: {}", AssetGuid.ToString());
        return false;
    }

    bool ManagerImpl::RequestUnload(
        const Handle& AssetGuid)
    {
        for (auto Package : Storage::GetPackages(true, true))
        {
            if (Package->RemoveAsset(AssetGuid, false))
            {
                return true;
            }
        }

        NEON_ERROR_TAG("Asset", "Asset not found: {}", AssetGuid.ToString());
        return false;
    }
} // namespace Neon::Asset