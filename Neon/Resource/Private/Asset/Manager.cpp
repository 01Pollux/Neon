#include <ResourcePCH.hpp>
#include <Asset/Manager.hpp>
#include <Asset/Storage.hpp>
#include <Asset/Packages/Directory.hpp>

//

#include <Asset/Handlers/TextFileHandler.hpp>

//

#include <Log/Logger.hpp>

namespace Neon::AAsset
{
    Manager::Manager() :
        m_Storage(std::make_unique<Storage>())
    {
        RegisterStandardHandlers();
    }

    NEON_CLASS_MOVE_IMPL(Manager);

    Manager::~Manager() = default;

    //

    IPackage* Manager::Mount(
        UPtr<IPackage> Package)
    {
        auto PackagePtr = m_Packages.emplace_back(std::move(Package)).get();
        PackagePtr->Mount(m_Storage.get());
        return PackagePtr;
    }

    void Manager::Unmount(
        IPackage* Package)
    {
        for (auto Iter = m_Packages.begin(); Iter != m_Packages.end(); ++Iter)
        {
            if (Iter->get() == Package)
            {
                Package->Unmount(m_Storage.get());
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

    cppcoro::task<Ref<IAsset>> Manager::Load(
        const Handle& ResHandle)
    {
        return m_Storage->Load(ResHandle);
    }

    void Manager::Unload(
        const Handle& ResHandle)
    {
        return m_Storage->Unload(ResHandle);
    }

    //

    void Manager::RegisterStandardHandlers()
    {
        RegisterHandler<TextFileHandler>("TextFile");
    }
} // namespace Neon::AAsset