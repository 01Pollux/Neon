#pragma once

#include <fstream>
#include <Core/String.hpp>

namespace Neon::IO
{
    namespace BinaryStream
    {
        /// <summary>
        /// Get size of string in bytes
        /// </summary>
        template<typename _Ty, typename _Traits = std::char_traits<_Ty>>
        [[nodiscard]] constexpr static size_t Size(
            const std::basic_string<_Ty, _Traits>& String) noexcept
        {
            return sizeof(size_t) + String.size() * sizeof(_Ty);
        }

        /// <summary>
        /// Get size of string in bytes
        /// </summary>
        template<typename _Ty>
            requires std::is_standard_layout_v<_Ty>
        [[nodiscard]] constexpr static size_t Size(
            const _Ty&) noexcept
        {
            return sizeof(_Ty);
        }

        /// <summary>
        /// Get size of string in bytes
        /// </summary>
        template<typename _Ty>
            requires std::is_standard_layout_v<_Ty>
        [[nodiscard]] constexpr static size_t Size() noexcept
        {
            return sizeof(_Ty);
        }
    } // namespace BinaryStream

    template<typename _FSTy>
    class TBinaryStreamReader
    {
    public:
        TBinaryStreamReader() = default;
        TBinaryStreamReader(
            _FSTy& Stream) :
            m_Stream(&Stream)
        {
        }

        [[nodiscard]] auto& Get() const noexcept
        {
            return (*m_Stream);
        }

        [[nodiscard]] auto& Get() noexcept
        {
            return (*m_Stream);
        }

        [[nodiscard]] auto operator->() const noexcept
        {
            return m_Stream;
        }

        [[nodiscard]] auto operator->() noexcept
        {
            return m_Stream;
        }

        [[nodiscard]] auto& operator*() const noexcept
        {
            return Get();
        }

        [[nodiscard]] auto& operator*() noexcept
        {
            return Get();
        }

        explicit operator bool() const noexcept
        {
            return m_Stream && *m_Stream;
        }

        /// <summary>
        /// Read bytes from the file.
        /// </summary>
        void ReadBytes(
            void*  Buffer,
            size_t Size)
        {
            m_Stream->read(std::bit_cast<char*>(Buffer), Size);
        }

        /// <summary>
        /// Read a value from the file.
        /// </summary>
        template<typename _Ty>
            requires std::is_standard_layout_v<_Ty>
        void Read(
            _Ty& Value)
        {
            if constexpr (std::endian::native == std::endian::big)
            {
                std::array<uint8_t, sizeof(_Ty)> Bytes;
                ReadBytes(Bytes.data(), Bytes.size());
                std::reverse(Bytes.begin(), Bytes.end());
                Value = std::bit_cast<_Ty>(Bytes);
            }
            else
            {
                ReadBytes(&Value, sizeof(_Ty));
            }
        }

        /// <summary>
        /// Read a string from the file.
        /// </summary>
        template<typename _Ty, typename _Traits = std::char_traits<_Ty>>
        void Read(
            std::basic_string<_Ty, _Traits>& String)
        {
            size_t Size = Read<size_t>();
            String.resize(Size);
            ReadBytes(String.data(), Size * sizeof(_Ty));
        }

        /// <summary>
        /// Read a value from the file.
        /// </summary>
        template<typename _Ty>
        _Ty Read()
        {
            // either read string or value
            _Ty Value{};
            Read(Value);
            return Value;
        }

    private:
        _FSTy* m_Stream = nullptr;
    };
    using BinaryStreamReader = TBinaryStreamReader<std::istream>;
    using BinaryFileReader   = TBinaryStreamReader<std::ifstream>;

    template<typename _FSTy>
    class TBinaryStreamWriter
    {
    public:
        TBinaryStreamWriter() = default;
        TBinaryStreamWriter(
            _FSTy& Stream) :
            m_Stream(&Stream)
        {
        }
        [[nodiscard]] auto& Get() const noexcept
        {
            return (*m_Stream);
        }

        [[nodiscard]] auto& Get() noexcept
        {
            return (*m_Stream);
        }

        [[nodiscard]] auto operator->() const noexcept
        {
            return m_Stream;
        }

        [[nodiscard]] auto operator->() noexcept
        {
            return m_Stream;
        }

        [[nodiscard]] auto& operator*() const noexcept
        {
            return Get();
        }

        [[nodiscard]] auto& operator*() noexcept
        {
            return Get();
        }

        explicit operator bool() const noexcept
        {
            return m_Stream && *m_Stream;
        }

        /// <summary>
        /// Write bytes to the file.
        /// </summary>
        void WriteBytes(
            const void* Buffer,
            size_t      Size)
        {
            m_Stream->write(std::bit_cast<const char*>(Buffer), Size);
        }

        /// <summary>
        /// Read a value from the file.
        /// </summary>
        template<typename _Ty>
            requires std::is_standard_layout_v<_Ty>
        void Write(
            _Ty Value)
        {
            if constexpr (std::endian::native == std::endian::big)
            {
                std::array Bytes = std::bit_cast<std::array<uint8_t, sizeof(_Ty)>>(Value);
                std::reverse(Bytes.begin(), Bytes.end());
                WriteBytes(Bytes.data(), Bytes.size());
            }
            else
            {
                WriteBytes(&Value, sizeof(_Ty));
            }
        }

        /// <summary>
        /// Write a string to the file.
        /// </summary>
        template<typename _Ty, typename _Traits = std::char_traits<_Ty>>
        void Write(
            const std::basic_string<_Ty, _Traits>& String)
        {
            size_t Size = String.size();
            Write(Size);
            WriteBytes(String.data(), Size * sizeof(_Ty));
        }

    private:
        _FSTy* m_Stream = nullptr;
    };
    using BinaryStreamWriter = TBinaryStreamWriter<std::ostream>;
    using BinaryFileWriter   = TBinaryStreamWriter<std::ofstream>;
} // namespace Neon::IO