#include <ResourcePCH.hpp>
#include <Asset/Manager.hpp>
#include <Asset/Storage.hpp>
#include <Asset/Packages/Directory.hpp>

#include <Asset/Handlers/TextFileHandler.hpp>

#include <Log/Logger.hpp>

//

namespace views  = std::views;
namespace ranges = std::ranges;

//

namespace Neon::AAsset
{
    Manager::Manager() :
        m_Storage(std::make_unique<Storage>())
    {
        RegisterStandardHandlers();
    }

    Manager::~Manager() = default;

    //

    IPackage* Manager::Mount(
        UPtr<IPackage> Package)
    {
        std::scoped_lock Lock(m_PackagesMutex);

        // verify if package exists in m_Packages
#ifdef NEON_DEBUG
        bool Found = false;
        for (const auto& PackagePtr : m_Packages)
        {
            NEON_ASSERT(PackagePtr.get() != Package.get(), "Trying to mount a package that was already mounted");
        }
#endif

        auto PackagePtr = m_Packages.emplace_back(std::move(Package)).get();
        return PackagePtr;
    }

    void Manager::Unmount(
        IPackage* Package)
    {
        std::scoped_lock Lock(m_PackagesMutex);

        // verify if package exists in m_Packages
#ifdef NEON_DEBUG
        bool Found = false;
        for (const auto& PackagePtr : m_Packages)
        {
            if (PackagePtr.get() == Package)
            {
                Found = true;
                break;
            }
        }
        NEON_ASSERT(Found, "Trying to get assets from a package that was not mounted");
#endif

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
        std::unique_lock Lock(m_PackagesMutex);

        auto PackagesCopy = m_Packages |
                            views::transform([](const auto& Package)
                                             { return Package.get(); }) |
                            ranges::to<std::vector>();

        Lock.unlock();

        for (auto Package : PackagesCopy)
        {
            co_yield Package;
        }
    }

    cppcoro::generator<const AAsset::Handle&> Manager::GetPackageAssets(
        IPackage* Package) const noexcept
    {
#ifdef NEON_DEBUG
        {
            // verify if package exists in m_Packages
            std::scoped_lock Lock(m_PackagesMutex);
            bool             Found = false;
            for (const auto& PackagePtr : m_Packages)
            {
                if (PackagePtr.get() == Package)
                {
                    Found = true;
                    break;
                }
            }
            NEON_ASSERT(Found, "Trying to get assets from a package that was not mounted");
        }
#endif

        std::shared_lock AssetLock(Package->m_AssetsMutex);

        auto AssetsCopy = Package->GetAssets();
        for (const auto& Asset : AssetsCopy)
        {
            co_yield Asset;
        }
    }

    //

    void Manager::RegisterHandler(
        const StringU8&     Name,
        UPtr<IAssetHandler> Handler)
    {
        m_Storage->RegisterHandler(Name, std::move(Handler));
    }

    void Manager::UnregisterHandler(
        const StringU8& Name)
    {
        m_Storage->UnregisterHandler(Name);
    }

    //

    std::future<Ref<IAsset>> Manager::Load(
        IPackage*       Package,
        const Handle& ResHandle)
    {
        return m_Storage->Load(Package, ResHandle);
    }

    std::future<void> Manager::Flush(
        IPackage* Package)
    {
        return m_Storage->Flush(Package);
    }

    void Manager::Unload(
        const Handle& ResHandle)
    {
        return m_Storage->Unload(ResHandle);
    }

    //

    void Manager::RegisterStandardHandlers()
    {
        RegisterHandler<TextFileHandler>();
    }
} // namespace Neon::AAsset