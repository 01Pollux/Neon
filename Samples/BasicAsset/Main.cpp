#include <Runtime/EntryPoint.hpp>
#include <Runtime/Types/WorldRuntime.hpp>

using namespace Neon;

class AssetPackSample : public Runtime::DefaultGameEngine
{
public:
    void Initialize(
        const Config::EngineConfig& Config) override
    {
        DefaultGameEngine::Initialize(Config);

        TestPacks();
    }

private:
    void TestPacks();
};

NEON_MAIN(Argc, Argv)
{
    Config::EngineConfig Config{
        .Window = {
            .Title      = STR("Test Engine"),
            .Windowed   = true,
            .Fullscreen = false },
    };
    return RunEngine<AssetPackSample>(Config);
}

//

#include <Asset/Manager.hpp>
#include <Asset/Packages/Directory.hpp>

//

void AssetPackSample::TestPacks()
{
    auto Manager = std::make_unique<AAsset::Manager>();

    //

    Manager->Mount(std::make_unique<AAsset::PackageDirectory>("Test/Directory1"));
    Manager->Mount(std::make_unique<AAsset::PackageDirectory>("Test/Directory2"));

    //

    auto Handle0 = AAsset::Handle::FromString("00000000-0000-0000-0000-000000000000");
    auto Handle1 = AAsset::Handle::FromString("00000000-0000-0000-0000-000000000001");
    auto Handle2 = AAsset::Handle::FromString("00000000-0000-0000-0000-000000000002");
    auto Handle3 = AAsset::Handle::FromString("00000000-0000-0000-0000-000000000003");

    //

    Manager->LoadAsync(Handle0);
    Manager->LoadAsync(Handle1);

    //

    auto Asset1 = Manager->Load(Handle1);
    auto Asset2 = Manager->Load(Handle2);
    auto Asset0 = Manager->Load(Handle0);
    auto Asset3 = Manager->Load(Handle3);

    Manager.reset();
}