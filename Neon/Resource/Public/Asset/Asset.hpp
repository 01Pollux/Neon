#pragma once

#include <Asset/Handle.hpp>

namespace Neon::AAsset
{
    class IAsset
    {
    public:
        IAsset(
            const Handle& AssetGuid) :
            m_AssetGuid(AssetGuid)
        {
        }

        virtual ~IAsset() = default;

        /// <summary>
        /// Gets the asset guid.
        /// </summary>
        [[nodiscard]] const Handle& GetGuid() const noexcept
        {
            return m_AssetGuid;
        }

    protected:
        const Handle m_AssetGuid;
    };
} // namespace Neon::AAsset