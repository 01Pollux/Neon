#include <ResourcePCH.hpp>
#include <Asset/Packages/Directory.hpp>
#include <Asset/PackageDescriptor.hpp>

#include <Log/Logger.hpp>

namespace Neon::AAsset
{
    PackageDirectory::PackageDirectory(
        const std::filesystem::path& Path)
    {
        if (Path.empty() || Path.native().starts_with(STR("..")))
        {
            NEON_ERROR_TAG("Asset", "Trying to mount a directory that is not valid");
            return;
        }

        if (!std::filesystem::exists(Path))
        {
            NEON_ERROR_TAG("Asset", "Trying to mount a directory that does not exist");
            return;
        }

        if (!std::filesystem::is_directory(Path))
        {
            NEON_ERROR_TAG("Asset", "Trying to mount a directory that is not a directory");
            return;
        }

        PackageDescriptor Descriptor(std::ifstream(Path / "Descriptor.ini"));
        for (auto& [Key, Path] : Descriptor.GetPaths())
        {
            printf("%s\n", Path.c_str());
        }
    }
} // namespace Neon::AAsset