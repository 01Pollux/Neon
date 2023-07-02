#include <PakCPCH.hpp>
#include <Parser/JsonReader.hpp>
#include <fstream>

#include <Resource/Packs/ZipPack.hpp>
#include <Resource/Types/Logger.hpp>
#include <Parser/Manager.hpp>

#include <Parser/Handlers/Logger.hpp>
#include <Parser/Handlers/TextFile.hpp>
#include <Parser/Handlers/Shader.hpp>
#include <Parser/Handlers/RootSignature.hpp>
#include <Parser/Handlers/Texture.hpp>

#include <Log/Logger.hpp>

//

namespace PakC
{
    using namespace Neon;

    JsonHandler::JsonHandler() :
        m_ResourceManager(std::make_unique<CustomResourceManager>())
    {
        RegisterTypes();
    }

    void JsonHandler::Parse(
        const StringU8& Path)
    {
        NEON_INFO("Parsing: '{}'", Path);

        std::ifstream File(Path);
        if (!File)
        {
            NEON_ERROR("Failed to open file");
            return;
        }

        std::error_code Error;

        auto Json = boost::json::parse(File, Error);
        File.close();

        if (Error)
        {
            NEON_ERROR("Failed to parse JSON: []-{}", Error.value(), Error.message());
            return;
        }

        if (!Json.is_object())
        {
            NEON_ERROR("Failed to parse JSON: Expected object");
            return;
        }

        auto& Object = Json.as_object();

        auto Assets     = Object.find("Assets");
        auto PackType   = Object.find("Type");
        auto ExportPath = Object.find("Export Path");

        if (PackType == Object.end() || !PackType->value().is_string())
        {
            NEON_ERROR("Failed to find pack type");
            return;
        }

        if (Assets == Object.end() || !Assets->value().is_array())
        {
            NEON_ERROR("Failed to find assets");
            return;
        }

        if (ExportPath == Object.end() || !ExportPath->value().is_string())
        {
            NEON_ERROR("Failed to find export path");
            return;
        }

        Neon::Asset::IAssetPack* Pack;

        StringU8 PackName(PackType->value().as_string());
        switch (Neon::StringUtils::Hash(PackName))
        {
        case Neon::StringUtils::Hash("ZipAsset"):
        {
            Pack = m_ResourceManager->NewPack<Neon::Asset::ZipAssetPack>(Path);
            break;
        }
        default:
        {
            NEON_ERROR("Failed to find pack type: '{}'", PackName);
            return;
        }
        }

        size_t Count = 0;
        for (auto& Element : Assets->value().as_array() |
                                 std::views::transform([](auto& Element) -> decltype(auto)
                                                       { return Element.as_object(); }))
        {
            Count++;
            auto Type = Element.find("Type");
            if (Type == Element.end() || !Type->value().is_string())
            {
                NEON_ERROR("Failed to find type at position: {}", Count);
                continue;
            }

            auto ForceHandle = Element.find("Handle");

            Neon::Asset::AssetHandle Handle =
                ForceHandle != Element.end() && ForceHandle->value().is_string()
                    ? Asset::AssetHandle::FromString(StringU8(ForceHandle->value().as_string()))
                    : Asset::AssetHandle::Random();

            auto Tag = Element.find("Tag");
            if (Tag != Element.end() && Tag->value().is_string())
            {
                NEON_INFO("Loading: '{}'", StringU8(Tag->value().as_string()));
            }
            else
            {
                NEON_INFO("Loading: {} -- Handle: {}", Count, Handle.ToString());
            }

            auto TypeHandler = m_AssetResources.find(StringU8(Type->value().as_string()));
            if (TypeHandler == m_AssetResources.end())
            {
                NEON_ERROR("Failed to find handler for type: '{}'", StringU8(Type->value().as_string()));
                continue;
            }

            AssetResourcePtr Asset;
            try
            {
                if (!(Asset = TypeHandler->second(Element)))
                {
                    throw std::runtime_error("null asset returned.");
                }
            }
            catch (const std::exception& Exception)
            {
                NEON_ERROR("Failed to load asset: {}", Exception.what());
                continue;
            }

            NEON_INFO("Successfully loaded asset");
            Pack->SaveAsync(Handle, Asset);
        }

        Pack->ExportAsync(StringU8(ExportPath->value().as_string()));
    }

    void JsonHandler::RegisterTypes()
    {
        m_AssetResources["LoggerAsset"]        = &Handler::LoadLoggerResource;
        m_AssetResources["TextFileAsset"]      = &Handler::LoadTextResource;
        m_AssetResources["ShaderLibraryAsset"] = &Handler::LoadShaderResource;
        m_AssetResources["RootSignatureAsset"] = &Handler::LoadRootSignatureResource;
        m_AssetResources["TextureAsset"]       = &Handler::LoadTextureResource;
    }
} // namespace PakC