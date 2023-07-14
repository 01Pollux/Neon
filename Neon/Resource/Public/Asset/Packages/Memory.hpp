#pragma once

#include <Asset/Package.hpp>
#include <filesystem>

namespace Neon::AAsset
{
    class PackageInMemory : public IPackage
    {
    public:
        PackageInMemory();

    private:
    };
} // namespace Neon::AAsset