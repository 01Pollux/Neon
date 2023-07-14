#include <ResourcePCH.hpp>
#include <Asset/Manager.hpp>

#include <Asset/Packages/Directory.hpp>

#include <filesystem>

#include <Log/Logger.hpp>

namespace Neon::AAsset
{
    IPackage* Manager::Mount(
        UPtr<IPackage> Package)
    {
        return m_Packages.emplace_back(std::move(Package)).get();
    }

    void Manager::Unmount(
        IPackage* Package)
    {
        for (auto Iter = m_Packages.begin(); Iter != m_Packages.end(); ++Iter)
        {
            if (Iter->get() == Package)
            {
                m_Packages.erase(Iter);
                return;
            }
        }

        NEON_WARNING_TAG("Asset", "Trying to unmount a package that was not mounted");
    }

    cppcoro::generator<IPackage*> Manager::GetPackages() const noexcept
    {
        for (const auto& Package : m_Packages)
        {
            co_yield Package.get();
        }
    }

    //

    void Manager::LoadAsync(
        const Handle& Handle)
    {
    }

    Ref<IAsset> Manager::Load(
        const Handle& Handle)
    {
        return {};
    }

    void Manager::Unload(
        const Handle& Handle)
    {
    }
} // namespace Neon::AAsset