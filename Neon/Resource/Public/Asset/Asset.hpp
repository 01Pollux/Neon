#pragma once

#include <Asset/Handle.hpp>

namespace Neon::AAsset
{
    class IAsset
    {
    public:
        IAsset(
            const Handle& NewHandle = Handle::Random());

        NEON_CLASS_NO_COPYMOVE(IAsset);

        virtual ~IAsset() = default;

        /// <summary>
        /// Gets the handle of the asset.
        /// </summary>
        const Handle& GetGuid() const noexcept;

    private:
        Handle m_Handle;
    };
} // namespace Neon::AAsset