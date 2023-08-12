#include <Runtime/EntryPoint.hpp>
#include <Asset/Packs/Directory.hpp>
#include "Engine.hpp"

using namespace Neon;

void Test()
{
    flecs::world World;
    struct MyComponent
    {
        struct SubComponent
        {
        };

        std::unique_ptr<int> ptr;
    };

    World.component<MyComponent::SubComponent>();
    World.component<MyComponent>();

    auto Ent = World.entity("Test");
    Ent.set<MyComponent>({}); // This will crash
}

NEON_MAIN(Argc, Argv)
{
    Test();
    Config::EngineConfig Config{
        .Window = {
            .Title      = STR("Flappy Bird"),
            .Windowed   = true,
            .Fullscreen = false },
    };

    Config.Renderer.Device.EnableDebugLayer = false;
    //   Config.Renderer.Swapchain.VSync         = false;
    //  Config.Renderer.Device.EnableGpuBasedValidation   = true;
    Config.Renderer.Device.Descriptors.Frame_Resource = 262'144;
    Config.Resource.AssetPackages.emplace_back(std::make_unique<Asset::DirectoryAssetPackage>("Contents"));
    return RunEngine<FlappyBirdClone>(std::move(Config));
}
