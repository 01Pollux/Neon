#pragma once

#include <Core/String.hpp>
#include <map>

namespace Neon::Asset
{
    class IResourceManager;
}

namespace Neon::Config
{
    struct ResourceConfig
    {
        Asset::IResourceManager*     Manager;
        std::map<StringU8, StringU8> Packs;
    };
} // namespace Neon::Config