#pragma once

#include <boost/uuid/uuid.hpp>

namespace Neon::Asset
{
    class AssetHandle
    {
    public:
        constexpr AssetHandle(
            const boost::uuids::uuid& Uid) :
            m_Uuid(Uid)
        {
        }

        /// <summary>
        /// Get underlying Uuid
        /// </summary>
        [[nodiscard]] const boost::uuids::uuid& Get() const noexcept;

        /// <summary>
        /// Get pack index containing asset
        /// </summary>
        [[nodiscard]] uint16_t GetPack() const noexcept;

    private:
        boost::uuids::uuid m_Uuid;
    };
} // namespace Neon::Asset