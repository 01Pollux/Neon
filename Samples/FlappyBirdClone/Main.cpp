#include <Runtime/EntryPoint.hpp>
#include <Asset/Packs/Directory.hpp>
#include "Engine.hpp"

using namespace Neon;

struct Shape
{
    int data = 2;
};

struct Rigidbody
{
    std::unique_ptr<int[]> underlying_data;

    Rigidbody(const Shape* Obj)
    {
        assert(Obj);
        underlying_data = std::make_unique<int[]>(Obj->data);
    }

    static void SetIn(flecs::entity Entity)
    {
        auto Obj = Entity.get<Shape>();
        Entity.set(Rigidbody{ Obj });
        // other manipulations...
    }
};

void OnUpdate(flecs::iter Iter)
{
    auto Ent = Iter.world().entity();
    Ent.set(Shape{ 5 });
    Rigidbody::SetIn(Ent);
}

void Test()
{
    flecs::world World;

    World.system()
        .kind(flecs::OnUpdate)
        .iter(
            [](flecs::iter Iter)
            {
                OnUpdate(Iter);
            });

    World.progress();
}

NEON_MAIN(Argc, Argv)
{
    Test();
    return 0;

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
