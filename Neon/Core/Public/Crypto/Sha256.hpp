#pragma once

/*
 * AUTHOR:	Aaron D. Gifford - http://www.aarongifford.com/
 *
 * Copyright (c) 2000-2001, Aaron D. Gifford
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTOR(S) ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTOR(S) BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: sha2.h,v 1.1 2001/11/08 00:02:01 adg Exp adg $
 */

#include <string>
#include <array>
#include <iosfwd>

namespace Neon::Crypto
{
    namespace Impl
    {
        static constexpr size_t SHA256_BLOCK_LENGTH         = 64;
        static constexpr size_t SHA256_DIGEST_LENGTH        = 32;
        static constexpr size_t SHA256_DIGEST_STRING_LENGTH = (SHA256_DIGEST_LENGTH * 2 + 1);

        typedef struct _SHA256_CTX
        {
            uint32_t state[8];
            uint64_t bitcount;
            uint8_t  buffer[SHA256_BLOCK_LENGTH];
        } SHA256_CTX;

        void  SHA256_Init(SHA256_CTX*);
        void  SHA256_Update(SHA256_CTX*, const uint8_t*, size_t);
        void  SHA256_Final(uint8_t[SHA256_DIGEST_LENGTH], SHA256_CTX*);
        char* SHA256_End(SHA256_CTX*, char[SHA256_DIGEST_STRING_LENGTH]);
        char* SHA256_Data(const uint8_t*, size_t, char[SHA256_DIGEST_STRING_LENGTH]);
    } // namespace Impl

    class Sha256
    {
    public:
        struct Bytes;

        Sha256();

        /// <summary>
        /// Append data to the hash.
        /// </summary>
        Sha256& Append(
            const void* Data,
            size_t      Size);

        /// <summary>
        /// Append data to the hash.
        /// </summary>
        Sha256& Append(
            std::istream& Stream,
            size_t        Size);

        /// <summary>
        /// Reset the hash.
        /// </summary>
        void Reset();

        /// <summary>
        /// Get the hash digest.
        /// </summary>
        [[nodiscard]] Bytes Digest();

    private:
        Impl::SHA256_CTX m_Context;
    };

    struct Sha256::Bytes : std::array<uint8_t, Impl::SHA256_DIGEST_STRING_LENGTH>
    {
        /// <summary>
        /// Convert to hex string
        /// </summary>
        [[nodiscard]] std::string ToString() const;
    };

    //

    /// <summary>
    /// Append string type to the hash.
    /// </summary>
    template<typename _CharTy, typename _CharTraits = std::char_traits<_CharTy>>
    inline Sha256& operator<<(
        Sha256&                                             Hash,
        const std::basic_string_view<_CharTy, _CharTraits>& String)
    {
        return Hash.Append(String.data(), String.size() * sizeof(typename _CharTraits::char_type));
    }

    /// <summary>
    /// Append string type to the hash.
    /// </summary>
    template<typename _CharTy, typename _CharTraits = std::char_traits<_CharTy>>
    inline Sha256& operator<<(
        Sha256&                                        Hash,
        const std::basic_string<_CharTy, _CharTraits>& String)
    {
        return Hash.Append(String.data(), String.size() * sizeof(typename _CharTraits::char_type));
    }

    /// <summary>
    /// Append standard layout type to the hash.
    /// </summary>
    template<typename _Ty>
        requires std::is_standard_layout_v<_Ty>
    inline Sha256& operator<<(
        Sha256& Hash,
        _Ty     Data)
    {
        return Hash.Append(std::bit_cast<uint8_t*>(std::addressof(Data)), sizeof(_Ty));
    }
} // namespace Neon::Crypto