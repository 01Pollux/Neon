#pragma once

#include <Core/Guid.hpp>

namespace Neon::Asset
{
    class AssetHandle
    {
    public:
        explicit AssetHandle(const Guid& Uid) :
            m_Uid(Uid)
        {
        }

        /// <summary>
        /// Get pack index containning asset
        /// </summary>
        [[nodiscard]] uint16_t GetPack() const noexcept;

    private:
        Guid m_Uid;
    };
} // namespace Neon::Asset