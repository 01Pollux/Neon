#include <ResourcePCH.hpp>
#include <Private/Asset/Manager.hpp>
#include <Private/Asset/Storage.hpp>
#include <Asset/Pack.hpp>

#include <Log/Logger.hpp>

namespace Neon::AAsset
{
    static UPtr<ManagerImpl> s_Instance;

    //

    void ManagerImpl::Initialize()
    {
        NEON_ASSERT(!s_Instance, "Asset Manager already initialized");
        s_Instance = std::make_unique<ManagerImpl>();
    }

    void ManagerImpl::Shutdown()
    {
        NEON_ASSERT(s_Instance, "Asset Manager not initialized");
        s_Instance.reset();
    }

    ManagerImpl* ManagerImpl::Get()
    {
        return s_Instance.get();
    }

    //

    std::future<Ptr<IAsset>> Manager::Load(
        const Handle& AssetGuid)
    {
        return s_Instance->Load(AssetGuid);
    }

    std::future<Ptr<IAsset>> Manager::Reload(
        const Handle& AssetGuid)
    {
        return s_Instance->Reload(AssetGuid);
    }

    void Manager::Unload(
        const Handle& AssetGuid)
    {
        s_Instance->Unload(AssetGuid);
    }

    //

    std::future<Ptr<IAsset>> ManagerImpl::Load(
        const Handle& AssetGuid)
    {
        for (auto Package : Storage::GetPackages(true))
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
        for (auto Package : Storage::GetPackages(true))
        {
            if (Package->RemoveAsset(AssetGuid))
            {
                break;
            }
        }
        return Load(AssetGuid);
    }

    bool ManagerImpl::Unload(
        const Handle& AssetGuid)
    {
        for (auto Package : Storage::GetPackages(true))
        {
            if (Package->RemoveAsset(AssetGuid))
            {
                return true;
            }
        }

        NEON_ERROR_TAG("Asset", "Asset not found: {}", AssetGuid.ToString());
        return false;
    }
} // namespace Neon::AAsset