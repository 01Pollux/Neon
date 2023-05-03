#pragma once

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>

namespace Neon::Asset
{
    struct AssetHandle : boost::uuids::uuid
    {
        [[nodiscard]] static AssetHandle Random()
        {
            return { boost::uuids::random_generator_pure()() };
        }
    };
} // namespace Neon::Asset