#pragma once

#include <Core/Neon.hpp>
#include <Resource/Handle.hpp>

namespace Neon::Asset
{
    class AssetHandle;
    class IAssetResourceHandler;

    class IAssetResource
    {
    public:
        IAssetResource(
            const AssetHandle& Handle) :
            m_Handle(Handle)
        {
        }

        NEON_CLASS_NO_COPYMOVE(IAssetResource);

        virtual ~IAssetResource() = default;

        [[nodiscard]] const AssetHandle& GetHandle() const noexcept;

    private:
        AssetHandle m_Handle;
    };
} // namespace Neon::Asset