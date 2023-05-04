#include <CorePCH.hpp>
#include <Core/SHA256.hpp>
#include <numeric>

namespace Neon
{
    static constexpr std::array<uint32_t, 64> SHA256_Table = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    SHA256::SHA256() :
        m_Data{
            0x6a09e667,
            0xbb67ae85,
            0x3c6ef372,
            0xa54ff53a,
            0x510e527f,
            0x9b05688c,
            0x1f83d9ab,
            0x5be0cd19
        }
    {
    }

    void SHA256::Append(
        std::istream& Stream,
        size_t        Size)
    {
        m_DataSize += Size;

        char c;
        // Flush out remaining chunks
        if (m_ChunkSize)
        {
            for (; m_ChunkSize < m_LastChunk.size() && Size > 0; Size--, m_ChunkSize++)
            {
                Stream >> c;
                m_LastChunk[m_ChunkSize] = static_cast<uint8_t>(c);
            }

            // If we processed every byte in stream, but there is room left in temp buffer
            if (!Size)
                return;

            ProcessChunk(m_LastChunk.data());
            m_ChunkSize = 0;
        }

        // Process in granularity
        while (Size >= 64)
        {
            Stream.read(std::bit_cast<char*>(m_LastChunk.data()), m_LastChunk.size());
            ProcessChunk(m_LastChunk.data());
            Size -= 64;
        }

        Stream.read(std::bit_cast<char*>(m_LastChunk.data() + m_ChunkSize), Size);
        m_ChunkSize += uint8_t(Size);
    }

    void SHA256::Append(
        const uint8_t* Data,
        size_t         Size)
    {
        m_DataSize += Size;

        // Flush out remaining chunks
        if (m_ChunkSize)
        {
            for (; m_ChunkSize < m_LastChunk.size() && Size > 0; Size--, m_ChunkSize++)
            {
                m_LastChunk[m_ChunkSize] = *Data++;
            }

            // If we processed every byte in stream, but there is room left in temp buffer
            if (!Size)
                return;

            ProcessChunk(m_LastChunk.data());
            m_ChunkSize = 0;
        }

        // Process in granularity
        while (Size >= 64)
        {
            ProcessChunk(Data);
            Data += 64;
            Size -= 64;
        }

        std::copy_n(Data, Size, m_LastChunk.data() + m_ChunkSize);
        m_ChunkSize += uint8_t(Size);
    }

    void SHA256::Append(
        const std::string& data)
    {
        Append(std::bit_cast<const uint8_t*>(data.c_str()), data.size());
    }

    auto SHA256::Digest() -> Bytes
    {
        m_LastChunk[m_ChunkSize++] = 0x80;
        std::memset(m_LastChunk.data() + m_ChunkSize, 0, m_LastChunk.size() - m_ChunkSize);

        /* If there isn't enough space to fit int64, pad chunk with zeroes and prepare next chunk */
        if (m_ChunkSize > 56)
        {
            ProcessChunk(m_LastChunk.data());
            m_LastChunk = {};
        }

        /* Add total size as big-endian int64 x8 */
        uint64_t Size = m_DataSize * 8;
        for (int i = 8; i > 0; --i)
        {
            m_LastChunk[55 + i] = Size & 0xFF;
            Size >>= 8;
        }

        ProcessChunk(m_LastChunk.data());

        Bytes Hash{};
        for (int i = 0; i < 8; i++)
        {
            Hash[i * 4]     = (m_Data[i] >> 24) & 0xFF;
            Hash[i * 4 + 1] = (m_Data[i] >> 16) & 0xFF;
            Hash[i * 4 + 2] = (m_Data[i] >> 8) & 0xFF;
            Hash[i * 4 + 3] = m_Data[i] & 0xFF;
        }
        return Hash;
    }

    //

    void SHA256::ProcessChunk(
        const uint8_t* Chunk)
    {
        std::array<uint32_t, 64> w;

        for (int i = 0; i < 16; ++i)
        {
            w[i] = (uint32_t)Chunk[0] << 24 | (uint32_t)Chunk[1] << 16 | (uint32_t)Chunk[2] << 8 | (uint32_t)Chunk[3];
            Chunk += 4;
        }

        for (int i = 16; i < 64; ++i)
        {
            uint32_t s0 = std::rotr(w[i - 15], 7) ^ std::rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
            uint32_t s1 = std::rotr(w[i - 2], 17) ^ std::rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i]        = w[i - 16] + s0 + w[i - 7] + s1;
        }

        auto tv = m_Data;
        for (int i = 0; i < 64; ++i)
        {
            uint32_t S1 = std::rotr(tv[4], 6) ^ std::rotr(tv[4], 11) ^ std::rotr(tv[4], 25);
            uint32_t ch = (tv[4] & tv[5]) ^ (~tv[4] & tv[6]);

            uint32_t temp1 = tv[7] + S1 + ch + SHA256_Table[i] + w[i];
            uint32_t S0    = std::rotr(tv[0], 2) ^ std::rotr(tv[0], 13) ^ std::rotr(tv[0], 22);

            uint32_t maj   = (tv[0] & tv[1]) ^ (tv[0] & tv[2]) ^ (tv[1] & tv[2]);
            uint32_t temp2 = S0 + maj;

            tv[7] = tv[6];
            tv[6] = tv[5];
            tv[5] = tv[4];
            tv[4] = tv[3] + temp1;
            tv[3] = tv[2];
            tv[2] = tv[1];
            tv[1] = tv[0];
            tv[0] = temp1 + temp2;
        }

        std::transform(tv.begin(), tv.end(), m_Data.begin(), m_Data.begin(), std::plus<>{});
    }

    std::string SHA256::Bytes::ToString() const
    {
        constexpr const char Hex[] = "0123456789abcdef";

        std::string Hash(64, '\0');
        for (size_t i = 0; i < size(); i++)
        {
            uint8_t c       = data()[i];
            Hash[i * 2]     = static_cast<char>(Hex[c >> 4]);
            Hash[i * 2 + 1] = static_cast<char>(Hex[c & 0xF]);
        }
        return Hash;
    }
} // namespace Neon