#pragma once

#include <string>
#include <boost/uuid/uuid.hpp>

namespace Neon::Asset
{
    struct AssetHandle : boost::uuids::uuid
    {
        /// <summary>
        /// Generate asset handle from string
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] std::string ToString() const;

        /// <summary>
        /// Generate asset handle from string
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] std::wstring ToWideString() const;

        /// <summary>
        /// Generate asset handle from string
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] static AssetHandle FromString(
            const std::string& Id);

        /// <summary>
        /// Generate random asset handle
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] static AssetHandle Random();
    };
} // namespace Neon::Asset