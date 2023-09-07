#pragma once

#include <filesystem>

namespace Neon::FileSystem
{
    /// <summary>
    /// Convert the path to a unix path.
    /// </summary>
    static void MakeUnixPath(
        std::filesystem::path& Path)
    {
#if NEON_PLATFORM_WINDOWS
        auto Str = std::move(Path.native());
        std::replace(Str.begin(), Str.end(), L'\\', L'/');
        Path = std::move(Str);
#endif
    }

    /// <summary>
    /// Convert the path to a unix path.
    /// </summary>
    [[nodiscard]] static std::filesystem::path ConvertToUnixPath(
        const std::filesystem::path& Path)
    {
#if NEON_PLATFORM_WINDOWS
        auto Copy = Path;
        MakeUnixPath(Copy);
        return Copy;
#else
        return Path;
#endif
    }

} // namespace Neon::FileSystem