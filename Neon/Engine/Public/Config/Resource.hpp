#pragma once

#include <Core/String.hpp>
#include <map>

namespace Neon::Config
{
    struct ResourceConfig
    {
        std::map<StringU8, StringU8> Packs;
    };
} // namespace Neon::Config