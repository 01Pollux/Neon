#include <PakCPCH.hpp>
#include <Parser/Handlers/Shader.hpp>
#include <fstream>

#include <RHI/Shader.hpp>
#include <Resource/Types/Shader.hpp>
#include <Log/Logger.hpp>

namespace PakC::Handler
{
    using namespace Neon;

    AssetResourcePtr LoadShaderResource(
        const boost::json::object& Object)
    {
        Neon::RHI::ShaderCompileDesc Desc;

        std::string FileBuffer;
        {
            auto Source = std::string(Object.at("Source").as_string());

            std::ifstream File(Source);
            if (!File)
            {
                throw std::runtime_error("Failed to open shader source file.");
            }
            std::stringstream Buffer;
            Buffer << File.rdbuf();
            FileBuffer = Buffer.str();
        }

        Desc.SourceCode = FileBuffer;
        Desc.EntryPoint = StringUtils::Transform<String>(Object.at("EntryPoint").as_string());

        switch (StringUtils::Hash(std::string(Object.at("Stage").as_string())))
        {
        case StringUtils::Hash("Compute"):
            Desc.Stage = RHI::ShaderStage::Compute;
            break;
        case StringUtils::Hash("Vertex"):
            Desc.Stage = RHI::ShaderStage::Vertex;
            break;
        case StringUtils::Hash("Pixel"):
            Desc.Stage = RHI::ShaderStage::Pixel;
            break;
        case StringUtils::Hash("Geometry"):
            Desc.Stage = RHI::ShaderStage::Geometry;
            break;
        case StringUtils::Hash("Hull"):
            Desc.Stage = RHI::ShaderStage::Hull;
            break;
        case StringUtils::Hash("Domain"):
            Desc.Stage = RHI::ShaderStage::Domain;
            break;
        }

        auto ProfileView = Object.at("Profile").as_string() |
                           std::views::split('.') |
                           std::views::take(2) |
                           std::views::transform([](auto&& Range)
                                                 { return StringU8(Range.begin(), Range.end()); });

        auto ProfileIter = ProfileView.begin();

        uint8_t Major = std::stoi(*ProfileIter++);
        uint8_t Minor = std::stoi(*ProfileIter);

        switch (Major)
        {
        case 6:
        {
            if (Minor >= 0 && Minor <= 6)
            {
                Desc.Profile = RHI::ShaderProfile(uint8_t(RHI::ShaderProfile::SP_6_0) + Minor);
            }
            break;
        }
        default:
        {
            throw std::runtime_error("Invalid shader profile.");
        }
        }

        if (auto Defines = Object.find("Defines"); Defines != Object.end() && Defines->value().is_object())
        {
            for (auto& [Key, Value] : Defines->value().as_object())
            {
                Desc.Defines.emplace_back(
                    StringUtils::Transform<String>(std::string(Key)),
                    StringUtils::Transform<String>(std::string(Value.as_string())));
            }
        }

        if (auto Flags = Object.find("Flags"); Flags != Object.end() && Flags->value().is_array())
        {
            std::map<boost::json::string, RHI::EShaderCompileFlags> FlagMap{
                { "Debug", RHI::EShaderCompileFlags::Debug }
            };
            for (auto& Flag : Flags->value().as_array())
            {
                auto Iter = FlagMap.find(Flag.as_string());
                if (Iter != FlagMap.end())
                {
                    Desc.Flags.Set(Iter->second);
                }
            }
        }

        auto Shader = RHI::IShader::Create(Desc);
        return std::make_shared<Asset::ShaderAsset>(Ptr<RHI::IShader>(Shader));
    }
} // namespace PakC::Handler