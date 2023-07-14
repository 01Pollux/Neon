#pragma once

#include <Core/Neon.hpp>

namespace Neon::AAsset
{
    /// <summary>
    /// A package is a collection of assets.
    /// It can be a file, a folder, a zip file...
    /// </summary>
    class IPackage
    {
    public:
        IPackage();
        NEON_CLASS_NO_COPYMOVE(IPackage);
        virtual ~IPackage() = default;
    };
} // namespace Neon::AAsset