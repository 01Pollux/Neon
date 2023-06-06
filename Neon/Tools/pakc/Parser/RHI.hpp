#pragma once

#include <Core/String.hpp>
#include <RHI/Resource/View.hpp>

namespace PakC
{
    [[nodiscard]] Neon::RHI::ESamplerFilter ConvertSamplerFilter(
        const Neon::StringU8& Filter);

    [[nodiscard]] Neon::RHI::ESamplerMode ConvertSamplerAddressMode(
        const Neon::StringU8& AddressMode);

    [[nodiscard]] Neon::RHI::ECompareFunc ConvertCompareFunc(
        const Neon::StringU8& CompareFunction);
} // namespace PakC