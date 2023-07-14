#include <ResourcePCH.hpp>
#include <Asset/Storage.hpp>
#include <Asset/Package.hpp>

#include <Log/Logger.hpp>

namespace Neon::AAsset
{
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
            auto Asset           = It->second->Load(Handle);
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