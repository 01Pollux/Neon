#pragma once

#include <Asset/Asset.hpp>
#include <optional>

namespace Neon::Asset
{
    class TextFileAsset : public IAsset
    {
    public:
        class Handler;

        using IAsset::IAsset;

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
        mutable std::variant<String, StringU8> m_Text = "";
    };
} // namespace Neon::Asset