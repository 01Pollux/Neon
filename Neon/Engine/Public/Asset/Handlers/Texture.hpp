#pragma once

#include <Resource/Asset.hpp>
#include <Resource/Handler.hpp>
#include <RHI/Resource/Resource.hpp>

namespace Neon::Asset
{
    class TextureAsset : public IAssetResource
    {
        friend class Handler;

    public:
        TextureAsset(
            const RHI::TextureRawImage& ImageInfo = {},
            bool                        Owning    = false);

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

    public:
        class Handler : public IAssetResourceHandler
        {
        public:
            bool CanCastTo(
                const Ptr<IAssetResource>& Resource) override;

            Ptr<IAssetResource> Load(
                IAssetPack*    Pack,
                IO::InArchive& Archive,
                size_t         DataSize) override;

            void Save(
                IAssetPack*                Pack,
                const Ptr<IAssetResource>& Resource,
                IO::OutArchive&            Archive) override;
        };

    private:
        RHI::TextureRawImage       m_ImageInfo;
        std::unique_ptr<uint8_t[]> m_ImageData;
    };
} // namespace Neon::Asset