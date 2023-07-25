#include <Runtime/EntryPoint.hpp>
#include <Runtime/Types/WorldRuntime.hpp>
#include <Asset/Packs/Directory.hpp>

using namespace Neon;

class FlappyBirdClone : public Runtime::DefaultGameEngine
{
public:
    void Initialize(
        Config::EngineConfig Config) override
    {
        DefaultGameEngine::Initialize(std::move(Config));
    }
};

NEON_MAIN(Argc, Argv)
{
    Config::EngineConfig Config{
        .Window = {
            .Title      = STR("Flappy Bird"),
            .Windowed   = true,
            .Fullscreen = false },
        //.Renderer{ .Device = { .EnableGpuBasedValidation = true } }
    };
    Config.Resource.AssetPackages.emplace_back(std::make_unique<Asset::DirectoryAssetPackage>("Contents"));
    return RunEngine<FlappyBirdClone>(std::move(Config));
}
