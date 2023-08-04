#include <Runtime/EntryPoint.hpp>
#include <Asset/Packs/Directory.hpp>
#include "Engine.hpp"

using namespace Neon;

NEON_MAIN(Argc, Argv)
{
    Config::EngineConfig Config{
        .Window = {
            .Title      = STR("Flappy Bird"),
            .Windowed   = true,
            .Fullscreen = false },
        //.Renderer{ .Device = { .EnableGPUDebugger = false } }
        //.Renderer{ .Device = { .EnableDebugLayer = false } }
        //.Renderer{ .Device = { .EnableGpuBasedValidation = true } }
    };
    Config.Resource.AssetPackages.emplace_back(std::make_unique<Asset::DirectoryAssetPackage>("Contents"));
    return RunEngine<FlappyBirdClone>(std::move(Config));
}
