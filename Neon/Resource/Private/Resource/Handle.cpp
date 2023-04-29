#include <ResourcePCH.hpp>
#include <Resource/Handle.hpp>

namespace Neon::Asset
{
    uint16_t AssetHandle::GetPack() const noexcept
    {
        uint16_t Pack;
        m_Uid.GetInv(std::bit_cast<uint8_t*>(&Pack), sizeof(Pack));
        return Pack;
    }
} // namespace Neon::Asset