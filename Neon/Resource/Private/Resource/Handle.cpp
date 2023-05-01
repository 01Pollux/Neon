#include <ResourcePCH.hpp>
#include <Resource/Handle.hpp>

namespace Neon::Asset
{
    const boost::uuids::uuid& AssetHandle::Get() const noexcept
    {
        return m_Uuid;
    }

    uint16_t AssetHandle::GetPack() const noexcept
    {
        uint16_t Pack;
        std::copy_n(m_Uuid.end() - sizeof(uint16_t), sizeof(uint16_t), std::bit_cast<uint8_t*>(&Pack));
        return Pack;
    }
} // namespace Neon::Asset