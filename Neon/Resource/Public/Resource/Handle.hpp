#pragma once

#include <Core/String.hpp>
#include <boost/uuid/uuid.hpp>

namespace Neon::Asset
{
    struct AssetHandle : boost::uuids::uuid
    {
        /// <summary>
        /// Generate asset handle from string
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] StringU8 ToString() const;

        /// <summary>
        /// Generate asset handle from string
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] String ToWideString() const;

        /// <summary>
        /// Generate asset handle from string
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] static AssetHandle FromString(
            const StringU8& Id);

        /// <summary>
        /// Generate random asset handle
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] static AssetHandle Random();
    };
} // namespace Neon::Asset