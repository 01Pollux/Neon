#include <PakCPCH.hpp>
#include <Parser/Handlers/Shader.hpp>
#include <fstream>
#include <filesystem>

#include <Resource/Types/Shader.hpp>
#include <Log/Logger.hpp>

namespace PakC::Handler
{
    using namespace Neon;

    AssetResourcePtr LoadShaderResource(
        const boost::json::object& Object)
    {
        auto ShaderLib = std::make_shared<Asset::ShaderLibraryAsset>();

        for (auto& M : Object.at("Modules").as_array())
        {
        }

        std::ifstream File;
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

            std::stringstream Buffer;
            Buffer << File.rdbuf();
            File.close();

            ShaderLib->SetModule(ModId, std::move(ModName), Buffer.str());
        }

        return ShaderLib;
    }
} // namespace PakC::Handler