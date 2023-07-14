#include <ResourcePCH.hpp>
#include <Asset/Packages/Directory.hpp>

#include <Log/Logger.hpp>

namespace Neon::AAsset
{
    PackageDirectory::PackageDirectory(
        const std::filesystem::path& Path)
    {
        if (Path.has_relative_path())
        {
            NEON_ERROR_TAG("Asset", "Trying to mount a directory that is relative");
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
    }
} // namespace Neon::AAsset