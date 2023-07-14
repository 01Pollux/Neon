#include <ResourcePCH.hpp>
#include <Asset/Storage.hpp>
#include <Asset/Package.hpp>
#include <Asset/Handler.hpp>

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
        const Handle& Handle)
    {
    }

    Ref<IAsset> Storage::Load(
        const Handle& Handle)
    {
        if (auto It = m_AssetCache.find(Handle); It != m_AssetCache.end())
        {
            return It->second;
        }

        if (auto It = m_AssetsInPackage.find(Handle); It != m_AssetsInPackage.end())
        {
            auto Asset           = It->second->Load(this, Handle);
            m_AssetCache[Handle] = Asset;
        }

        NEON_ERROR_TAG("Asset", "Trying to load non-existing asset");
        return {};
    }

    void Storage::Unload(
        const Handle& Handle)
    {
    }

    //

    void Storage::Reference(
        IPackage*     Package,
        const Handle& Handle)
    {
        m_AssetsInPackage[Handle] = Package;
    }

    void Storage::Unreference(
        IPackage*     Package,
        const Handle& Handle)
    {
        m_AssetsInPackage.erase(Handle);
        m_AssetCache.erase(Handle);
    }
} // namespace Neon::AAsset