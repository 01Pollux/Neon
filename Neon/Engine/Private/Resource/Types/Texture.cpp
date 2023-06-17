#include <EnginePCH.hpp>
#include <Resource/Types/Texture.hpp>
#include <boost/serialization/access.hpp>

#include <IO/Archive.hpp>

namespace boost::serialization
{
    template<typename _Archive>
    void load(
        _Archive&                   Archive,
        Neon::RHI::TextureRawImage& ImageInfo,
        boost::serialization::version_type)
    {
        std::underlying_type_t<Neon::RHI::TextureRawImage::Format> Format;

        Archive >> Format;
        Archive >> ImageInfo.Size;
        if (ImageInfo.Size)
        {
            ImageInfo.Data = NEON_NEW uint8_t[ImageInfo.Size];
            Archive.load_binary(
                ImageInfo.Data,
                ImageInfo.Size);
        }

        ImageInfo.Type = static_cast<Neon::RHI::TextureRawImage::Format>(Format);
    }

    template<typename _Archive>
    void save(
        _Archive&                         Archive,
        const Neon::RHI::TextureRawImage& ImageInfo,
        boost::serialization::version_type)
    {
        Archive << std::to_underlying(ImageInfo.Type);
        Archive << ImageInfo.Size;
        if (ImageInfo.Size)
        {
            Archive.save_binary(
                ImageInfo.Data,
                ImageInfo.Size);
        }
    }
} // namespace boost::serialization
BOOST_SERIALIZATION_SPLIT_FREE(Neon::RHI::TextureRawImage);

namespace Neon::Asset
{
    TextureAsset::TextureAsset(
        const RHI::TextureRawImage& ImageInfo,
        bool                        Owning)
    {
        SetImageInfo(ImageInfo, Owning);
    }

    const RHI::TextureRawImage& TextureAsset::GetImageInfo() const
    {
        return m_ImageInfo;
    }

    void TextureAsset::SetImageInfo(
        const RHI::TextureRawImage& ImageInfo,
        bool                        Owning)
    {
        if (Owning)
        {
            m_ImageInfo = ImageInfo;
            m_ImageData.reset(std::bit_cast<uint8_t*>(ImageInfo.Data));
        }
        else
        {
            if (ImageInfo.Size > m_ImageInfo.Size)
            {
                m_ImageData = std::make_unique<uint8_t[]>(ImageInfo.Size);
            }
            m_ImageInfo = ImageInfo;
            if (m_ImageInfo.Data)
            {
                std::copy_n(ImageInfo.Data, ImageInfo.Size, m_ImageData.get());
                m_ImageInfo.Data = m_ImageData.get();
            }
        }
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
        RHI::TextureRawImage ImageInfo;

        Archive& ImageInfo;
        return std::make_shared<TextureAsset>(ImageInfo, true);
    }

    void TextureAsset::Handler::Save(
        IAssetPack*,
        const Ptr<IAssetResource>& Resource,
        IO::OutArchive&            Archive)
    {
        auto  Texture   = static_cast<TextureAsset*>(Resource.get());
        auto& ImageInfo = Texture->GetImageInfo();

        Archive& ImageInfo;
    }
} // namespace Neon::Asset