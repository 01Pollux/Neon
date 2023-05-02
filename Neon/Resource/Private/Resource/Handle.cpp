#include <ResourcePCH.hpp>
#include <Resource/Handle.hpp>

namespace Neon::Asset
{
    uint16_t AssetHandle::GetPack() const noexcept
    {
        uint16_t Pack;
        std::copy_n(end() - sizeof(uint16_t), sizeof(uint16_t), std::bit_cast<uint8_t*>(&Pack));
        return Pack;
    }

    void AssetHandle::SetPack(
        uint16_t Index) noexcept
    {
        std::copy_n(std::bit_cast<uint8_t*>(&Index), sizeof(uint16_t), end() - sizeof(uint16_t));
    }
} // namespace Neon::Asset