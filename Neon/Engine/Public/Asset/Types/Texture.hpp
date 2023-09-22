#pragma once

#include <Asset/Asset.hpp>
#include <RHI/Resource/Resource.hpp>

namespace Neon::Asset
{
    class TextureAsset : public IAsset
    {
    public:
        class Handler;

        TextureAsset(
            const RHI::TextureRawImage& ImageInfo,
            bool                        Owning,
            const Handle&               AssetGuid,
            StringU8                    Path);

        /// <summary>
        /// Get texture image info.
        /// </summary>
        [[nodiscard]] const RHI::TextureRawImage& GetImageInfo() const;

        /// <summary>
        /// Set texture image info.
        /// </summary>
        void SetImageInfo(
            const RHI::TextureRawImage& ImageInfo,
            bool                        Owning = false);

        /// <summary>
        /// Get or load the texture.
        /// </summary>
        [[nodiscard]] const RHI::SSyncGpuResource& GetTexture() const;

    private:
        RHI::TextureRawImage       m_ImageInfo;
        std::unique_ptr<uint8_t[]> m_ImageData;
        RHI::SSyncGpuResource      m_Texture;
    };
} // namespace Neon::Asset