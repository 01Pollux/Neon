#pragma once

#include <Asset/Asset.hpp>
#include <Core/String.hpp>

namespace Neon::AAsset
{
    // We will test out child assets with this.
    class StringFileAsset : public IAsset
    {
    public:
        StringFileAsset(
            StringU8      Text,
            const Handle& Handle = Handle::Random());

        /// <summary>
        /// Get the text from the asset.
        /// </summary>
        [[nodiscard]] const StringU8& GetText() const;

        /// <summary>
        /// Set the text of the asset.
        /// </summary>
        void SetText(
            StringU8 Text);

    private:
        StringU8 m_Text;
    };
} // namespace Neon::AAsset
