#include <ResourcePCH.hpp>
#include <Private/Asset/Manager.hpp>

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
        s_Instance->Load(AssetGuid);
    }

    std::future<Ptr<IAsset>> Manager::Reload(
        const Handle& AssetGuid)
    {
        s_Instance->Reload(AssetGuid);
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
        return std::future<Ptr<IAsset>>();
    }

    std::future<Ptr<IAsset>> ManagerImpl::Reload(
        const Handle& AssetGuid)
    {
        return std::future<Ptr<IAsset>>();
    }

    void ManagerImpl::Unload(
        const Handle& AssetGuid)
    {
    }
} // namespace Neon::AAsset