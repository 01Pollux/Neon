#include <ResourcePCH.hpp>
#include <Asset/Manager.hpp>

#include <Asset/Packages/Directory.hpp>

#include <filesystem>

#include <Log/Logger.hpp>

namespace Neon::AAsset
{
    PackageHandle Manager::Mount(
        const StringU8& Path,
        MountType       Type)
    {
        size_t Hndl = StringUtils::Hash(Path);

        auto& Package = m_Packages[Hndl];
        if (!Package)
        {
            std::filesystem::path FsPath(Path);
            if (!std::filesystem::exists(FsPath))
            {
                NEON_ERROR("Package '{}' does not exist", Path);
                return 0;
            }

            switch (Type)
            {
            case Manager::MountType::Directory:
                if (!std::filesystem::is_directory(FsPath))
                {
                    NEON_ERROR("Package '{}' is not a directory", Path);
                    return 0;
                }
                Package = std::make_unique<PackageDirectory>(Path);
                break;

            case Manager::MountType::Zip:
                if (!std::filesystem::is_regular_file(FsPath))
                {
                    NEON_ERROR("Package '{}' is not a file", Path);
                    return 0;
                }
            case Manager::MountType::Database:
                NEON_ASSERT(false, "Unimplemented");
                break;
            default:
                NEON_ERROR("Package '{}' has an invalid mount type", Path);
                break;
            }

            if (!std::filesystem::is_directory(FsPath))
            {
            }
            else if (!std::filesystem::is_regular_file(FsPath))
            {
            }
        }
        else
        {
            NEON_WARNING("Package '{}' was already mounted", Path);
        }

        return Hndl;
    }

    void Manager::Unmount(
        PackageHandle Package)
    {
        if (!m_Packages.erase(Package))
        {
            NEON_WARNING("Package '{}' was not mounted", Package);
        }
    }

    Ref<IPackage> Manager::GetPackage(
        PackageHandle Package)
    {
        auto Iter = m_Packages.find(Package);
        if (Iter == m_Packages.end())
        {
            NEON_WARNING("Package '{}' was not mounted", Package);
            return {};
        }

        return Iter->second;
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