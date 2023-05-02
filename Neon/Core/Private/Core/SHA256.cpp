#include <CorePCH.hpp>
#include <Core/SHA256.hpp>

namespace Neon
{
    SHA256::SHA256() :
        m_BlockLen(0),
        m_Bitlen(0)
    {
        m_State[0] = 0x6a09e667;
        m_State[1] = 0xbb67ae85;
        m_State[2] = 0x3c6ef372;
        m_State[3] = 0xa54ff53a;
        m_State[4] = 0x510e527f;
        m_State[5] = 0x9b05688c;
        m_State[6] = 0x1f83d9ab;
        m_State[7] = 0x5be0cd19;
    }

    void SHA256::Append(uint8_t data)
    {
        m_Data[m_BlockLen++] = data;
        if (m_BlockLen == 64)
        {
            transform();

            // End of the block
            m_Bitlen += 512;
            m_BlockLen = 0;
        }
    }

    void SHA256::Append(const uint8_t* data, size_t length)
    {
        for (size_t i = 0; i < length; i++)
        {
            Append(data[i]);
        }
    }

    void SHA256::Append(
        const std::string& data)
    {
        Append(reinterpret_cast<const uint8_t*>(data.c_str()), data.size());
    }

    auto SHA256::Digest() -> Bytes
    {
        Bytes hash{};

        pad();
        revert(hash);

        return hash;
    }

    uint32_t SHA256::choose(uint32_t e, uint32_t f, uint32_t g)
    {
        return (e & f) ^ (~e & g);
    }

    uint32_t SHA256::majority(uint32_t a, uint32_t b, uint32_t c)
    {
        return (a & (b | c)) | (b & c);
    }

    uint32_t SHA256::sig0(uint32_t x)
    {
        return std::rotr(x, 7) ^ std::rotr(x, 18) ^ (x >> 3);
    }

    uint32_t SHA256::sig1(uint32_t x)
    {
        return std::rotr(x, 17) ^ std::rotr(x, 19) ^ (x >> 10);
    }

    void SHA256::transform()
    {
        uint32_t maj, xorA, ch, xorE, sum, newA, newE, m[64];
        uint32_t state[8];

        for (uint8_t i = 0, j = 0; i < 16; i++, j += 4)
        { // Split data in 32 bit blocks for the 16 first words
            m[i] = (m_Data[j] << 24) | (m_Data[j + 1] << 16) | (m_Data[j + 2] << 8) | (m_Data[j + 3]);
        }

        for (uint8_t k = 16; k < 64; k++)
        { // Remaining 48 blocks
            m[k] = SHA256::sig1(m[k - 2]) + m[k - 7] + SHA256::sig0(m[k - 15]) + m[k - 16];
        }

        for (uint8_t i = 0; i < 8; i++)
        {
            state[i] = m_State[i];
        }

        for (uint8_t i = 0; i < 64; i++)
        {
            maj  = SHA256::majority(state[0], state[1], state[2]);
            xorA = std::rotr(state[0], 2) ^ std::rotr(state[0], 13) ^ std::rotr(state[0], 22);

            ch = choose(state[4], state[5], state[6]);

            xorE = std::rotr(state[4], 6) ^ std::rotr(state[4], 11) ^ std::rotr(state[4], 25);

            sum  = m[i] + K[i] + state[7] + ch + xorE;
            newA = xorA + maj + sum;
            newE = state[3] + sum;

            state[7] = state[6];
            state[6] = state[5];
            state[5] = state[4];
            state[4] = newE;
            state[3] = state[2];
            state[2] = state[1];
            state[1] = state[0];
            state[0] = newA;
        }

        for (uint8_t i = 0; i < 8; i++)
        {
            m_State[i] += state[i];
        }
    }

    void SHA256::pad()
    {
        uint64_t i   = m_BlockLen;
        uint8_t  end = m_BlockLen < 56 ? 56 : 64;

        m_Data[i++] = 0x80; // Append a bit 1
        while (i < end)
        {
            m_Data[i++] = 0x00; // Pad with zeros
        }

        if (m_BlockLen >= 56)
        {
            transform();
            memset(m_Data, 0, 56);
        }

        // Append to the padding the total message's length in bits and transform.
        m_Bitlen += m_BlockLen * 8;
        m_Data[63] = uint8_t(m_Bitlen);
        m_Data[62] = uint8_t(m_Bitlen >> 8);
        m_Data[61] = uint8_t(m_Bitlen >> 16);
        m_Data[60] = uint8_t(m_Bitlen >> 24);
        m_Data[59] = uint8_t(m_Bitlen >> 32);
        m_Data[58] = uint8_t(m_Bitlen >> 40);
        m_Data[57] = uint8_t(m_Bitlen >> 48);
        m_Data[56] = uint8_t(m_Bitlen >> 56);
        transform();
    }

    void SHA256::revert(std::array<uint8_t, 32>& hash)
    {
        // SHA uses big endian byte ordering
        // Revert all bytes
        for (uint8_t i = 0; i < 4; i++)
        {
            for (uint8_t j = 0; j < 8; j++)
            {
                hash[i + (j * 4)] = (m_State[j] >> (24 - i * 8)) & 0x000000ff;
            }
        }
    }
} // namespace Neon