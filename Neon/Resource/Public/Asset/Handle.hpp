#pragma once

#include <Core/String.hpp>
#include <boost/uuid/uuid.hpp>

namespace boost::serialization
{
    class access;
}

namespace Neon::AAsset
{
    class IAsset;
    struct Handle : boost::uuids::uuid
    {
        static const Handle Null;

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
        [[nodiscard]] static Handle FromString(
            const StringU8& Id);

        /// <summary>
        /// Generate random asset handle
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] static Handle Random();

        friend class boost::serialization::access;
        template<typename _Archive>
        [[nodiscard]] void serialize(
            _Archive& Archive,
            uint32_t  Version)
        {
            Archive& data;
        }
    };
} // namespace Neon::AAsset

namespace std
{
    template<>
    struct hash<Neon::AAsset::Handle>
    {
        size_t operator()(
            const Neon::AAsset::Handle& Value) const noexcept;
    };
} // namespace std