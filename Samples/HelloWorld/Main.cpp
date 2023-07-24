#include <Runtime/EntryPoint.hpp>
#include <Runtime/Types/WorldRuntime.hpp>
#include <Asset/Packs/Directory.hpp>

using namespace Neon;

class RuntimeSample : public Runtime::DefaultGameEngine
{
public:
    void Initialize(
        Config::EngineConfig Config) override
    {
        DefaultGameEngine::Initialize(std::move(Config));
        // RegisterInterface<Runtime::IEngineRuntime, Runtime::EngineWorldRuntime>();
    }
};

NEON_MAIN(Argc, Argv)
{
    Config::EngineConfig Config{
        .Window = {
            .Title      = STR("Test Engine"),
            .Windowed   = true,
            .Fullscreen = false },
        //.Renderer{ .Device = { .EnableGpuBasedValidation = true } }
    };
    Config.Resource.AssetPackages.emplace_back(std::make_unique<Asset::DirectoryAssetPackage>("Contents"));
    return RunEngine<RuntimeSample>(std::move(Config));
}
