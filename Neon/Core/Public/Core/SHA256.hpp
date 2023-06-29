#pragma once

// https://github.com/LekKit/sha256

#include <string>
#include <array>
#include <memory>
#include <IO/BinaryFile.hpp>

namespace Neon
{
    class SHA256
    {
    public:
        struct Bytes : std::array<uint8_t, 32>
        {
        public:
            /// <summary>
            /// Convert to hex string
            /// </summary>
            [[nodiscard]] std::string ToString() const;
        };

        SHA256();

        /// <summary>
        /// Reset internal state
        /// </summary>
        void Reset();

        /// <summary>
        /// Append raw data from binary stream
        /// </summary>
        void Append(
            IO::BinaryStreamReader Stream,
            size_t                 Size);

        /// <summary>
        /// Append raw data
        /// </summary>
        void Append(
            const uint8_t* Data,
            size_t         Size);

        /// <summary>
        /// Append UTF-8 string
        /// </summary>
        void Append(
            const std::string& Data);

        /// <summary>
        /// Append UTF-16 string
        /// </summary>
        void Append(
            const std::wstring& Data);

        /// <summary>
        /// Append pod type
        /// </summary>
        template<typename _Ty>
            requires std::is_standard_layout_v<_Ty>
        void Append(
            _Ty Data)
        {
            Append(std::bit_cast<uint8_t*>(std::addressof(Data)), sizeof(_Ty));
        }

        /// <summary>
        /// Get digest of appended data
        /// </summary>
        [[nodiscard]] Bytes Digest();

    private:
        /// <summary>
        /// Process 64-byte chunk of data
        /// </summary>
        void ProcessChunk(
            const uint8_t* Chunk);

    private:
        size_t                  m_DataSize = 0;
        std::array<uint32_t, 8> m_Data;
        std::array<uint8_t, 64> m_LastChunk;
        uint8_t                 m_ChunkSize = 0;
    };
} // namespace Neon