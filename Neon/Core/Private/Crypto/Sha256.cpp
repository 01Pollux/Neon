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
 */

#include <CorePCH.hpp>
#include <Crypto/Sha256.hpp>

namespace Neon::Crypto
{
    Sha256::Sha256()
    {
        Reset();
    }

    void Sha256::Reset()
    {
        Impl::SHA256_Init(&m_Context);
    }

    auto Sha256::Digest() -> Bytes
    {
        Bytes Result;
        Impl::SHA256_End(&m_Context, std::bit_cast<char*>(Result.data()));
        return Result;
    }

    Sha256& Sha256::Append(
        const void* Data,
        size_t      Size)
    {
        Impl::SHA256_Update(&m_Context, std::bit_cast<const uint8_t*>(Data), Size);
        return *this;
    }

    Sha256& Sha256::Append(
        std::istream& Stream,
        size_t        Size)
    {
        std::array<uint8_t, 512> TmpBuffer;
        while (Size)
        {
            size_t ReadSize = std::min(Size, TmpBuffer.size());
            Stream.read(reinterpret_cast<char*>(TmpBuffer.data()), ReadSize);
            Impl::SHA256_Update(&m_Context, TmpBuffer.data(), ReadSize);
            Size -= ReadSize;
        }
        return *this;
    }

    std::string Sha256::Bytes::ToString() const
    {
        return std::string(std::bit_cast<const char*>(data()), size() - 1);
    }
} // namespace Neon::Crypto