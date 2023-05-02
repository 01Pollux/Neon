#pragma once

#include <boost/uuid/uuid.hpp>

namespace Neon::Asset
{
    class AssetHandle : public boost::uuids::uuid
    {
    public:
        /// <summary>
        /// Get pack index containing asset
        /// </summary>
        [[nodiscard]] uint16_t GetPack() const noexcept;

        /// <summary>
        /// Set pack index containing asset
        /// </summary>
        [[nodiscard]] void SetPack(
            uint16_t Index) noexcept;
    };
} // namespace Neon::Asset