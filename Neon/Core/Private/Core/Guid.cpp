#include <CorePCH.hpp>
#include <Core/Guid.hpp>

#include <combaseapi.h>

namespace Neon
{
    Guid::Guid(
        bool NotNull) noexcept
    {
        if (NotNull)
        {
            GUID guid;
            do
            {
                CoCreateGuid(&guid);
            } while (guid == GUID{});

            auto StartData = std::bit_cast<const uint8_t*>(&guid);
            std::copy_n(StartData, sizeof(guid), m_Data.data());
        }
    }

    Guid::Guid(
        uint32_t                      Data0,
        uint16_t                      Data1,
        uint16_t                      Data2,
        const std::array<uint8_t, 8>& Data3)
    {
        // pack Data0 to the last 128 bits in m_Data
        // pack Data1 to the last 96 bits in m_Data
        // pack Data2 to the last 80 bits in m_Data
        // pack Data3 to the last 64 bits in m_Data

        size_t Offset = 0;
        Set(std::bit_cast<uint8_t*>(&Data0), sizeof(Data0), Offset);

        Offset += sizeof(Data0);
        Set(std::bit_cast<uint8_t*>(&Data1), sizeof(Data1), Offset);

        Offset += sizeof(Data1);
        Set(std::bit_cast<uint8_t*>(&Data2), sizeof(Data2), Offset);

        Offset += sizeof(Data2);
        Set(Data3.data(), sizeof(Data3), Offset);
    }

    bool Guid::IsNull() const noexcept
    {
        return *this == Guid{};
    }

    void Guid::Get(
        uint8_t* Data,
        size_t   DataSize,
        size_t   Offset) const noexcept
    {
        std::copy_n(m_Data.data() + Offset, DataSize, Data);
    }

    void Guid::GetInv(
        uint8_t* Data,
        size_t   DataSize,
        size_t   Offset) const noexcept
    {
        auto StartData = m_Data.data() + m_Data.size() - Offset - DataSize;
        std::copy_n(StartData, DataSize, Data);
    }

    void Guid::Set(
        const uint8_t* Data,
        size_t         DataSize,
        size_t         Offset) noexcept
    {
        std::copy_n(Data, DataSize, m_Data.data() + Offset);
    }

    void Guid::SetInv(
        const uint8_t* Data,
        size_t         DataSize,
        size_t         Offset) noexcept
    {
        auto StartData = m_Data.data() + m_Data.size() - Offset - DataSize;
        std::copy_n(Data, DataSize, StartData);
    }
} // namespace Neon