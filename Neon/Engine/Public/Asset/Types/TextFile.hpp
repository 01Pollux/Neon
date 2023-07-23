#pragma once

#include <Asset/Asset.hpp>

#include <Core/String.hpp>
#include <optional>

namespace Neon::Asset
{
    class TextFileAsset : public IAsset
    {
        class Handler;

    public:
        TextFileAsset(
            String        Text,
            const Handle& AssetGuid,
            StringU8      Path);

        TextFileAsset(
            StringU8      Text,
            const Handle& AssetGuid,
            StringU8      Path);

        /// <summary>
        /// Load string as utf8
        /// </summary>
        [[nodiscard]] const StringU8& AsUtf8() const;

        /// <summary>
        /// Get utf16 string
        /// </summary>
        [[nodiscard]] const String& AsUtf16() const;

        /// <summary>
        /// Set utf8 text
        /// </summary>
        [[nodiscard]] void SetText(
            const StringU8& Text);

        /// <summary>
        /// Set utf16 text
        /// </summary>
        [[nodiscard]] void SetText(
            const String& Text);

    private:
        String                          m_Utf16Text;
        mutable std::optional<StringU8> m_Utf8Text;
    };
} // namespace Neon::Asset