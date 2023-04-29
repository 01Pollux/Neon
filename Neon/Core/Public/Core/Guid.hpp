#pragma once

#include <cstdint>
#include <array>

namespace Neon
{
    class Guid
    {
    public:
        static const Guid NullGuid;

        explicit Guid(
            bool NotNull = false) noexcept;

        Guid(
            uint32_t                      Data0,
            uint16_t                      Data1,
            uint16_t                      Data2,
            const std::array<uint8_t, 8>& Data3);

        /// <summary>
        /// Check if guid is null
        /// </summary>
        [[nodiscard]] bool IsNull() const noexcept;

        /// <summary>
        /// Get data at offset from left to right
        /// </summary>
        void Get(
            uint8_t* Data,
            size_t   DataSize,
            size_t   Offset = 0) const noexcept;

        /// <summary>
        /// Get data at offset from right to left
        /// </summary>
        void GetInv(
            uint8_t* Data,
            size_t   DataSize,
            size_t   Offset = 0) const noexcept;

        /// <summary>
        /// Set data at offset from left to right
        /// </summary>
        void Set(
            const uint8_t* Data,
            size_t         DataSize,
            size_t         Offset = 0) noexcept;

        /// <summary>
        /// Set data at offset from right to left
        /// </summary>
        void SetInv(
            const uint8_t* Data,
            size_t         DataSize,
            size_t         Offset = 0) noexcept;

        auto operator<=>(const Guid&) const noexcept = default;

    private:
        std::array<uint8_t, 16> m_Data;
    };

    inline constexpr Guid Guid::NullGuid{ 0, 0, 0, {} };

} // namespace Neon
