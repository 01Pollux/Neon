#pragma once

#include <cstdint>
#include <array>

namespace Neon
{
    class Guid
    {
    public:
        static constexpr Guid NullGuid = Guid{ 0, 0, 0, {} };

        Guid(bool NotNull = false);

        constexpr Guid(
            uint32_t                      Data0,
            uint16_t                      Data1,
            uint16_t                      Data2,
            const std::array<uint8_t, 8>& Data3) :
            m_Data0(Data0),
            m_Data1({ Data1, Data2 }),
            m_Data2(Data3)
        {
        }

        /// <summary>
        /// Check if guid is null
        /// </summary>
        [[nodiscard]] bool IsNull() const noexcept;

        constexpr auto operator<=>(const Guid&) const noexcept = default;

    private:
        uint32_t                m_Data0{};
        std::array<uint16_t, 2> m_Data1{};
        std::array<uint8_t, 8>  m_Data2{};
    };
} // namespace Neon
