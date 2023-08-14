#include <Runtime/EntryPoint.hpp>
#include <Asset/Packs/Directory.hpp>

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
    return RunEngine<Runtime::DefaultGameEngine>(std::move(Config));
}
