#pragma once

#include <Resource/Asset.hpp>
#include <Resource/Handler.hpp>

namespace Neon::RHI
{
    class ITexture;
}

namespace Neon::Asset
{
    class TextureAsset : public IAssetResource
    {
        friend class Handler;

    public:
        TextureAsset(
            const Ptr<RHI::ITexture>& Texture = nullptr);

        /// <summary>
        /// Get loaded texture resource.
        /// </summary>
        [[nodiscard]] const Ptr<RHI::ITexture>& GetTexture() const;

        /// <summary>
        /// Set texture resource.
        /// </summary>
        void SetTexture(
            const Ptr<RHI::ITexture>& Texture);

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
        Ptr<RHI::ITexture> m_Texture;
    };
} // namespace Neon::Asset