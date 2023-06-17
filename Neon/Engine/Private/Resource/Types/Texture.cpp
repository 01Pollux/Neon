#include <EnginePCH.hpp>
#include <Resource/Types/Texture.hpp>

namespace Neon::Asset
{
    TextureAsset::TextureAsset(
        const Ptr<RHI::ITexture>& Texture) :
        m_Texture(Texture)
    {
    }

    const Ptr<RHI::ITexture>& TextureAsset::GetTexture() const
    {
        return m_Texture;
    }

    void TextureAsset::SetTexture(
        const Ptr<RHI::ITexture>& Texture)
    {
        m_Texture = Texture;
    }

    //

    bool TextureAsset::Handler::CanCastTo(
        const Ptr<IAssetResource>& Resource)
    {
        return dynamic_cast<TextureAsset*>(Resource.get());
    }

    Ptr<IAssetResource> TextureAsset::Handler::Load(
        IAssetPack*,
        IO::InArchive& Archive,
        size_t)
    {
        return nullptr;
    }

    void TextureAsset::Handler::Save(
        IAssetPack*,
        const Ptr<IAssetResource>& Resource,
        IO::OutArchive&            Archive)
    {
    }
} // namespace Neon::Asset