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
            StringU8      Text,
            const Handle& AssetGuid,
            StringU8      Path);

        /// <summary>
        /// Load string
        /// </summary>
        [[nodiscard]] const StringU8& Get() const;

        /// <summary>
        /// Set string
        /// </summary>
        [[nodiscard]] void Set(
            const StringU8& Text);

    private:
        StringU8 m_Text;
    };
} // namespace Neon::Asset