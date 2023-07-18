#include <ResourcePCH.hpp>
#include <Asset/Storage.hpp>
#include <Asset/Package.hpp>
#include <Asset/Handler.hpp>

#include <cppcoro/sync_wait.hpp>

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

    std::future<Ref<IAsset>> Storage::Load(
        IPackage*     Package,
        const Handle& ResHandle)
    {
        auto Task = [this, Package, ResHandle]() -> Ref<IAsset>
        {
            Ptr<IAsset> Asset;
            {
                Asset = Package->Load(this, ResHandle);
            }

            if (!Asset)
            {
                NEON_ERROR_TAG("Asset", "Asset '{}' does not exist", ResHandle.ToString());
            }

            return Asset;
        };

        return m_ThreadPool.enqueue(std::move(Task));
    }

    std::future<void> Storage::Flush(
        IPackage* Package)
    {
        auto Task = [this, Package]
        {
            Package->Flush(this);
        };
        return m_ThreadPool.enqueue(std::move(Task));
    }

    void Storage::Unload(
        const Handle& ResHandle)
    {
    }
} // namespace Neon::AAsset