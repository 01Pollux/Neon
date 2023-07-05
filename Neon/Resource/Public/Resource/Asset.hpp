#pragma once

#include <Core/Neon.hpp>
#include <Resource/Handle.hpp>

namespace Neon::Asset
{
    class IAssetResource : public std::enable_shared_from_this<IAssetResource>
    {
    public:
        IAssetResource() = default;

        NEON_CLASS_NO_COPYMOVE(IAssetResource);

        virtual ~IAssetResource() = default;
    };
} // namespace Neon::Asset