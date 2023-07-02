#include <PakCPCH.hpp>
#include <Parser/Handlers/Shader.hpp>
#include <fstream>
#include <filesystem>

#include <Resource/Types/Shader.hpp>
#include <Log/Logger.hpp>

namespace views  = std::views;
namespace ranges = std::ranges;

namespace PakC::Handler
{
    using namespace Neon;

    bool ValidateShader(
        Asset::ShaderModule*      ShaderModule,
        const boost::json::array& Validation)
    {
        for (auto& Object : Validation |
                                std::views::transform(
                                    [](const auto& Stage) -> decltype(auto)
                                    { return Stage.as_object(); }))
        {
            RHI::ShaderStage Stage;
            auto             StageName = StringU8(Object.at("Stage").as_string());

            switch (StringUtils::Hash(StageName))
            {
            case StringUtils::Hash("Compute"):
                Stage = RHI::ShaderStage::Compute;
                break;
            case StringUtils::Hash("Vertex"):
                Stage = RHI::ShaderStage::Vertex;
                break;
            case StringUtils::Hash("Pixel"):
                Stage = RHI::ShaderStage::Pixel;
                break;
            case StringUtils::Hash("Geometry"):
                Stage = RHI::ShaderStage::Geometry;
                break;
            case StringUtils::Hash("Hull"):
                Stage = RHI::ShaderStage::Hull;
                break;
            case StringUtils::Hash("Domain"):
                Stage = RHI::ShaderStage::Domain;
                break;
            default:
                NEON_ERROR("Failed to parse shader stage: {}", StageName);
                return false;
            }

            RHI::ShaderProfile Profile = RHI::ShaderProfile::SP_6_0;
            if (auto ProfileName = Object.if_contains("Profile"); ProfileName && ProfileName->is_string())
            {
                auto ProfileView = ProfileName->as_string() |
                                   views::split('.') |
                                   views::take(2) |
                                   views::transform([](auto&& Range)
                                                    { return StringU8(Range.begin(), Range.end()); }) |
                                   ranges::to<std::vector<StringU8>>();

                auto ProfileIter = ProfileView.begin();

                uint8_t Major = std::stoi(ProfileIter[0]);
                uint8_t Minor = ProfileView.size() > 1 ? std::stoi(ProfileIter[1]) : 0;

                switch (Major)
                {
                case 6:
                {
                    if (Minor >= 0 && Minor <= 6)
                    {
                        Profile = RHI::ShaderProfile(uint8_t(RHI::ShaderProfile::SP_6_0) + Minor);
                        break;
                    }
                }
                default:
                {
                    NEON_ERROR("Invalid shader profile {}", StringU8(ProfileName->as_string()));
                    return false;
                }
                }
            }

            RHI::ShaderMacros Macros;
            if (auto Defines = Object.if_contains("Defines"); Defines && Defines->is_object())
            {
                for (auto& [Key, Value] : Defines->as_object())
                {
                    Macros.Append(
                        StringUtils::Transform<String>(std::string(Key)),
                        StringUtils::Transform<String>(std::string(Value.as_string())));
                }
            }

            RHI::MShaderCompileFlags Flags;
            if (auto FlagsNames = Object.if_contains("Flags"); FlagsNames && FlagsNames->is_array())
            {
                for (auto& Flag : FlagsNames->as_array())
                {
                    switch (StringUtils::Hash(StringU8(Flag.as_string())))
                    {
                    case StringUtils::Hash("Debug"):
                    {
                        Flags.Set(RHI::EShaderCompileFlags::Debug);
                        break;
                    }
                    default:
                    {
                        NEON_ERROR("Invalid shader flag: {}", StringU8(Flag.as_string()));
                        return false;
                    }
                    }
                }
            }
            else
            {
                Flags = RHI::MShaderCompileFlags_Default;
            }

            if (!ShaderModule->LoadStage(Stage, Flags, Profile, Macros))
            {
                NEON_ERROR("Failed to load shader stage: {}", StageName);
                return false;
            }
        }

        return true;
    }

    AssetResourcePtr LoadShaderResource(
        const boost::json::object& Object)
    {
        auto ShaderLib = std::make_shared<Asset::ShaderLibraryAsset>();

        std::ifstream     File;
        std::stringstream Buffer;

        for (auto& Module : Object.at("Modules").as_array() |
                                std::views::transform(
                                    [](const auto& Module) -> decltype(auto)
                                    { return Module.as_object(); })

        )
        {
            auto ModId   = Asset::ShaderModuleId(Module.at("Id").to_number<uint32_t>());
            auto ModName = StringU8(Module.at("Name").as_string());
            auto ModPath = StringU8(Module.at("Path").as_string());

            File.open(ModPath);
            if (!File)
            {
                throw std::runtime_error("Failed to open shader source file.");
            }

            Buffer.str("");
            Buffer.clear();
            Buffer << File.rdbuf();
            File.close();

            ShaderLib->SetModule(ModId, std::move(ModName), Buffer.str());

            if (auto Validation = Module.if_contains("Validation"); Validation && Validation->is_array())
            {
                if (!ValidateShader(ShaderLib->LoadModule(ModId), Validation->as_array()))
                {
                    return nullptr;
                }
            }
        }

        return ShaderLib;
    }
} // namespace PakC::Handler