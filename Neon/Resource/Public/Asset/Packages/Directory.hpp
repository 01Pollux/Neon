#pragma once

#include <Asset/Package.hpp>
#include <filesystem>

namespace Neon::AAsset
{
    class PackageDirectory : public IPackage
    {
    public:
        PackageDirectory(
            const std::filesystem::path& Path);
    };
} // namespace Neon::AAsset