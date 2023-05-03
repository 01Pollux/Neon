#pragma once

#include <Core/Neon.hpp>
#include <Resource/Handle.hpp>

namespace Neon::Asset
{
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

        /// <summary>
        /// Get asset handle.
        /// </summary>
        [[nodiscard]] const AssetHandle& GetHandle() const noexcept
        {
            return m_Handle;
        }

    private:
        AssetHandle m_Handle;
    };
} // namespace Neon::Asset