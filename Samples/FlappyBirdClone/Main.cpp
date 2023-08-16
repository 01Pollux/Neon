#include <Runtime/EntryPoint.hpp>
#include <Asset/Packs/Directory.hpp>
#include "Engine.hpp"

using namespace Neon;

NEON_MAIN(Argc, Argv)
{
    Config::EngineConfig Config{
        .Window = {
            .Title = "Flappy Bird",
            .Size  = { 1280, 720 } }
    };

    Config.Renderer.Device.EnableDebugLayer = false;
    //   Config.Renderer.Swapchain.VSync         = false;
    //  Config.Renderer.Device.EnableGpuBasedValidation   = true;
    Config.Renderer.Device.Descriptors.Frame_Resource = 262'144;

    Config.Resource.AssetPackages.emplace_back(std::make_unique<Asset::DirectoryAssetPackage>("Contents"));
    return RunEngine<FlappyBirdClone>(std::move(Config));
}
