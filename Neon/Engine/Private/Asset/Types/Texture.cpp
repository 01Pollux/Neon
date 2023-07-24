#include <EnginePCH.hpp>
#include <Asset/Handlers/Texture.hpp>

#include <boost/serialization/access.hpp>

namespace Neon::Asset
{
    TextureAsset::TextureAsset(
        const RHI::TextureRawImage& ImageInfo,
        bool                        Owning,
        const Handle&               AssetGuid,
        StringU8                    Path) :
        IAsset(AssetGuid, std::move(Path))
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

    bool TextureAsset::Handler::CanHandle(
        const Ptr<IAsset>& Resource)
    {
        return dynamic_cast<TextureAsset*>(Resource.get());
    }

    Ptr<IAsset> TextureAsset::Handler::Load(
        std::istream& Stream,
        const Asset::DependencyReader&,
        const Handle&        AssetGuid,
        StringU8             Path,
        const AssetMetaData& LoaderData)
    {
        using ImageFormat = RHI::TextureRawImage::Format;

        constexpr std::array Extensions = {
            std::pair{ ".dds", ImageFormat::Dds },
            std::pair{ ".ico", ImageFormat::Ico },
            std::pair{ ".bmp", ImageFormat::Bmp },
            std::pair{ ".png", ImageFormat::Png },
            std::pair{ ".jpeg", ImageFormat::Jpeg },
            std::pair{ ".jpg", ImageFormat::Jpeg },
            std::pair{ ".jxr", ImageFormat::Jxr },
            std::pair{ ".tiff", ImageFormat::Tiff }
        };

        for (const auto& [Extension, Type] : Extensions)
        {
            if (Path.ends_with(Extension))
            {
                RHI::TextureRawImage ImageInfo{
                    .Type = Type
                };

                Stream.seekg(std::ios::end);
                ImageInfo.Size = Stream.tellg();
                if (!ImageInfo.Size)
                {
                    break;
                }

                Stream.seekg(std::ios::beg);
                ImageInfo.Data = NEON_NEW uint8_t[ImageInfo.Size];

                Stream.read(
                    std::bit_cast<char*>(ImageInfo.Data),
                    ImageInfo.Size);

                return std::make_shared<TextureAsset>(std::move(ImageInfo), true, AssetGuid, std::move(Path));
            }
        }
        return nullptr;
    }

    void TextureAsset::Handler::Save(
        std::iostream& Stream,
        DependencyWriter&,
        const Ptr<IAsset>& Asset,
        AssetMetaData&     LoaderData)
    {
        auto  Texture   = static_cast<TextureAsset*>(Asset.get());
        auto& ImageInfo = Texture->GetImageInfo();

        Stream.write(
            std::bit_cast<const char*>(ImageInfo.Data),
            ImageInfo.Size);
    }
} // namespace Neon::Asset