#pragma once

#include <Core/Neon.hpp>

#include <string>
#include <format>
#include <algorithm>

#define STR(x)   L##x
#define STRU8(x) StringU8(x)

namespace Neon
{
    using CharU8       = char;
    using StringU8     = std::string;
    using StringU8View = std::string_view;

    using Char       = wchar_t;
    using String     = std::wstring;
    using StringView = std::wstring_view;
} // namespace Neon

namespace Neon::StringUtils
{
    template<typename _Ty>
        requires std::is_same_v<_Ty, StringU8> || std::is_same_v<_Ty, String>
    static _Ty Empty = {};

    template<typename _To, typename _From>
    [[nodiscard]] constexpr _To Transform(const _From& Str) noexcept
    {
        // same type
        if constexpr (std::is_same_v<_To, _From>)
            return Str;
        // from const _char* to _string_view
        else if constexpr (std::is_pointer_v<_From>)
            return Transform<_To>(std::basic_string_view<std::remove_pointer_t<_From>>(Str));
        else
        {
            if (Str.empty())
                return {};
            else
            {
                // from bigger type to smaller type, eg: wstring to string, u32string to u8string, etc...
                if constexpr (sizeof(_From::value_type) > sizeof(_To::value_type))
                {
                    _To buf(Str.size(), 0);
                    std::transform(std::begin(Str), std::end(Str), std::begin(buf), [](const typename _From::value_type c)
                                   { return static_cast<typename _To::value_type>(c); });
                    return buf;
                }
                // from smaller type to bigger type, eg: string to wstring, etc...
                else
                    return { std::begin(Str), std::end(Str) };
            }
        }
    }

    // support for array of characters
    template<typename _To, typename _From, size_t _Size>
    [[nodiscard]] constexpr _To Transform(const _From (&Str)[_Size]) noexcept
    {
        if constexpr (std::is_same_v<typename _To::value_type, _From>)
            return _To{ Str };
        else if constexpr (_Size < 1)
            return _To{};
        else
        {
            _To ToStr{};
            ToStr.reserve(_Size);
            std::transform(std::begin(Str), std::end(Str), std::back_inserter(ToStr), [](_From c)
                           { return static_cast<typename _To::value_type>(c); });
            return ToStr;
        }
    }

    [[nodiscard]] constexpr String ToLower(String Str) noexcept
    {
        String LowStr;
        LowStr.reserve(Str.size());
        std::ranges::transform(Str, std::back_inserter(LowStr), [](wchar_t c)
                               { return static_cast<wchar_t>(std::tolower(c)); });
        return LowStr;
    }

    [[nodiscard]] constexpr StringU8 ToLower(StringU8 Str) noexcept
    {
        StringU8 LowStr;
        LowStr.reserve(Str.size());
        std::ranges::transform(Str, std::back_inserter(LowStr), [](char c)
                               { return static_cast<char>(std::tolower(c)); });
        return LowStr;
    }

    [[nodiscard]] constexpr String ToUpper(String Str) noexcept
    {
        String UpStr;
        UpStr.reserve(Str.size());
        std::ranges::transform(Str, std::back_inserter(UpStr), [](wchar_t c)
                               { return static_cast<wchar_t>(std::toupper(c)); });
        return UpStr;
    }

    [[nodiscard]] constexpr StringU8 ToUpper(StringU8 Str) noexcept
    {
        StringU8 UpStr;
        UpStr.reserve(Str.size());
        std::ranges::transform(Str, std::back_inserter(UpStr), [](char c)
                               { return static_cast<char>(std::toupper(c)); });
        return UpStr;
    }

    template<typename... _Args>
    [[nodiscard]] constexpr String Format(const StringView FormatStr, _Args&&... Args)
    {
        return std::vformat(FormatStr, std::make_wformat_args(Args...));
    }

    template<typename... _Args>
    [[nodiscard]] constexpr StringU8 Format(const StringU8View FormatStr, _Args&&... Args)
    {
        return std::vformat(FormatStr, std::make_format_args(Args...));
    }

    namespace Impl
    {
        // https://github.com/elanthis/constexpr-hash-demo/blob/master/test.cpp
        // FNV-1a constants

        static constexpr size_t _Hash_Basis = 14695981039346656037ULL;
        static constexpr size_t _Hash_Prime = 1099511628211ULL;
    } // namespace Impl

    // compile-time hash
    [[nodiscard]] constexpr size_t Hash(const char* Str)
    {
        size_t Hash = Impl::_Hash_Basis;
        for (size_t i = 0; Str[i]; i++)
        {
            Hash ^= Str[i];
            Hash *= Impl::_Hash_Prime;
        }
        return Hash;
    }

    // compile-time hash
    [[nodiscard]] constexpr size_t Hash(const wchar_t* Str)
    {
        size_t Hash = Impl::_Hash_Basis;
        for (size_t i = 0; Str[i]; i++)
        {
            Hash ^= Str[i];
            Hash *= Impl::_Hash_Prime;
        }
        return Hash;
    }

    // run-time hash
    [[nodiscard]] constexpr size_t Hash(const StringU8& Str)
    {
        size_t Hash = Impl::_Hash_Basis;
        for (char i : Str)
        {
            Hash ^= i;
            Hash *= Impl::_Hash_Prime;
        }
        return Hash;
    }
    // run-time hash
    [[nodiscard]] constexpr size_t Hash(const String& Str)
    {
        size_t Hash = Impl::_Hash_Basis;
        for (wchar_t i : Str)
        {
            Hash ^= i;
            Hash *= Impl::_Hash_Prime;
        }
        return Hash;
    }

    inline namespace Literals
    {
        constexpr size_t operator"" _hash(const char* Str, size_t)
        {
            return Hash(Str);
        }
        constexpr size_t operator"" _hash(const wchar_t* Str, size_t)
        {
            return Hash(Str);
        }
    } // namespace Literals
} // namespace Neon::StringUtils
