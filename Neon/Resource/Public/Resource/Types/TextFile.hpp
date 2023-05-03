#pragma once

#include <Core/String.hpp>

#include <Resource/Asset.hpp>
#include <Resource/Handler.hpp>

namespace Neon::Asset
{
    class TextFileAsset : public IAssetResource
    {
    public:
        TextFileAsset(
            const AssetHandle& Handle,
            String             Text = L"");

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

    public:
        class Handler : public IAssetResourceHandler
        {
        public:
            bool CanCastTo(
                const Ptr<IAssetResource>& Resource) override;

            size_t QuerySize(
                const Ptr<IAssetResource>& Resource) override;

            Ptr<IAssetResource> Load(
                const AssetHandle& Handle,
                const uint8_t*     Data,
                size_t             DataSize) override;

            void Save(
                const Ptr<IAssetResource>& Resource,
                uint8_t*                   Data,
                size_t                     DataSize) override;
        };

    private:
        String                          m_Utf16Text;
        mutable std::optional<StringU8> m_Utf8Text;
    };
} // namespace Neon::Asset