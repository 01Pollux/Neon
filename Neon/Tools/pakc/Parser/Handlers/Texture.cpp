#include <PakCPCH.hpp>
#include <Parser/Handlers/Texture.hpp>
#include <fstream>
#include <filesystem>

#include <RHI/Resource/Resource.hpp>
#include <Resource/Types/Texture.hpp>
#include <Log/Logger.hpp>

namespace views  = std::views;
namespace ranges = std::ranges;

namespace PakC::Handler
{
    using namespace Neon;

    std::map<StringU8, RHI::TextureRawImage::Format> s_TextureImageFormat = {
        { "DDS", RHI::TextureRawImage::Format::Dds },
        { "ICO", RHI::TextureRawImage::Format::Ico },
        { "BMP", RHI::TextureRawImage::Format::Bmp },
        { "PNG", RHI::TextureRawImage::Format::Png },
        { "JPG", RHI::TextureRawImage::Format::Jpeg },
        { "JPEG", RHI::TextureRawImage::Format::Jpeg },
        { "JXR", RHI::TextureRawImage::Format::Jxr },
        { "TIFF", RHI::TextureRawImage::Format::Tiff }
    };

    AssetResourcePtr LoadTextureResource(
        const boost::json::object& Object)
    {
        ValidateFields(Object, "Path");

        auto Path = std::filesystem::path(StringU8(Object.at("Path").as_string()));

        std::ifstream File(Path, std::ios::binary | std::ios::ate);
        if (!File)
        {
            throw std::runtime_error("Failed to open texture file.");
        }

        size_t Size   = File.tellg();
        auto   Buffer = std::make_unique<uint8_t[]>(Size);

        File.seekg(0, std::ios::beg);
        File.read(std::bit_cast<char*>(Buffer.get()), Size);
        File.close();

        std::optional<RHI::TextureRawImage::Format> ExpectedFormat, ActualFormat;

        if (Path.has_extension())
        {
            // convert to uppercase
            auto Extension = Path.extension().string() |
                             views::drop(1) | // drop the dot
                             views::transform([](char c)
                                              { return std::toupper(c); }) |
                             ranges::to<StringU8>();

            auto FormatType = s_TextureImageFormat.find(Extension);
            if (FormatType != s_TextureImageFormat.end())
            {
                ActualFormat = FormatType->second;
            }
        }

        if (auto Format = Object.if_contains("Format"); Format && Format->is_string())
        {
            auto FormatType = s_TextureImageFormat.find(StringU8(Format->as_string()));
            if (FormatType != s_TextureImageFormat.end())
            {
                ExpectedFormat = FormatType->second;
            }
        }

        if (!ExpectedFormat && !ActualFormat)
        {
            throw std::runtime_error("Failed to determine texture format.");
        }

        if (ExpectedFormat && ActualFormat && *ExpectedFormat != *ActualFormat)
        {
            throw std::runtime_error("Texture format mismatch.");
        }

        RHI::TextureRawImage::Format FormatToUse = ExpectedFormat ? *ExpectedFormat : *ActualFormat;

        RHI::TextureRawImage ImageInfo{
            .Data = Buffer.release(),
            .Size = Size,
            .Type = FormatToUse
        };

        return std::make_shared<Asset::TextureAsset>(ImageInfo, true);
    }
} // namespace PakC::Handler