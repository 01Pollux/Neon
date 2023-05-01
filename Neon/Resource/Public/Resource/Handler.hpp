#pragma once

#include <Core/Neon.hpp>

namespace Neon::Asset
{
    class IAssetResource;

    class IAssetResourceHandler
    {
    public:
        /// <summary>
        /// Load data from bytes
        /// </summary>
        Ptr<IAssetResource> Load(
            const uint8_t* Data,
            size_t         DataSize);
    };
} // namespace Neon::Asset