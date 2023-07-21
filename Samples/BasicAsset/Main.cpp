#include <Runtime/EntryPoint.hpp>
#include <Runtime/Types/WorldRuntime.hpp>

#include <Asset/Storage.hpp>
#include <Asset/Asset.hpp>
#include <Asset/Handler.hpp>
#include <Asset/Packs/Directory.hpp>

//

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

//

namespace views  = std::views;
namespace ranges = std::ranges;

//

using namespace Neon;

class AssetPackSample : public Runtime::DefaultGameEngine
{
public:
    void Initialize(
        const Config::EngineConfig& Config) override
    {
        DefaultGameEngine::Initialize(Config);

        AAsset::Storage::Initialize();

        RegisterManager();

        SaveSimple();
        LoadSimple();

        // SaveDeps();
        // LoadDeps();

        AAsset::Storage::Shutdown();

        exit(0);
        return;
    }

private:
    void RegisterManager();

    void SaveSimple();
    void LoadSimple();

    // void SaveDeps();
    // void LoadDeps();
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

class StringAndChildAsset : public AAsset::IAsset
{
    friend class StringAndChildHandler;

public:
    StringAndChildAsset(
        StringU8                            Text,
        const AAsset::Handle&               Handle   = AAsset::Handle::Random(),
        std::span<Ptr<StringAndChildAsset>> Children = {}) :
        IAsset(Handle),
        m_Text(std::move(Text)),
        m_Children(Children.begin(), Children.end())
    {
    }

    [[nodiscard]] const StringU8& GetText() const
    {
        return m_Text;
    }

    void SetText(
        StringU8 Text)
    {
        m_Text = std::move(Text);
    }

private:
    StringU8                              m_Text;
    std::vector<Ptr<StringAndChildAsset>> m_Children;
};

class StringAndChildHandler : public AAsset::IAssetHandler
{
public:
    static constexpr const char* HandlerName = "StringAndChild";

    bool CanHandle(
        const Ptr<AAsset::IAsset>& Asset) override
    {
        return std::dynamic_pointer_cast<StringAndChildAsset>(Asset) != nullptr;
    }

    Ptr<AAsset::IAsset> Load(
        std::ifstream&               Stream,
        const AAsset::Handle&        AssetGuid,
        const AAsset::AssetMetaData& LoaderData) override
    {
        return nullptr;
    }

    void Save(
        std::ofstream&             Stream,
        const Ptr<AAsset::IAsset>& Asset,
        AAsset::AssetMetaData&     LoaderData) override
    {
        StringAndChildAsset* StringAndChild = static_cast<StringAndChildAsset*>(Asset.get());

        boost::archive::text_oarchive Archive(Stream, boost::archive::no_header | boost::archive::no_tracking);
        Archive << StringAndChild->GetText();

        LoaderData.put("Archive", true);
    }
};

void AssetPackSample::RegisterManager()
{
    AAsset::Storage::RegisterHandler<StringAndChildHandler>();

    //

    AAsset::Storage::Mount(std::make_unique<AAsset::DirectoryAssetPackage>("Test"));
}

static const char* TextTest = R"(
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nullam euismod, nisl eget ultricies ultrices, nunc nisl ultricies nunc, quis ultric
ies nisl nisl eget nisl. Nullam euismod, nisl eget ultricies ultrices, nunc nisl ultricies nunc, quis ultricies nisl nisl eget nisl. Nullam eu
ismod, nisl eget ultricies ultrices, nunc nisl ultricies nunc, quis ultricies nisl nisl eget nisl. Nullam euismod, nisl eget ultricies ultric
)";

static constexpr uint32_t c_AssetCount = 100;

void AssetPackSample::SaveSimple()
{
    std::vector<std::future<void>> Tasks;
    Tasks.reserve(c_AssetCount);

    auto t0 = std::chrono::high_resolution_clock::now();

    for (uint32_t i = 0; i < c_AssetCount; ++i)
    {
        Tasks.emplace_back(AAsset::Storage::AddAsset(
            { .Asset = std::make_shared<StringAndChildAsset>(TextTest),
              .Path  = StringUtils::Format("File/{}.txt", i) }));
    }

    for (auto& Task : Tasks)
    {
        Task.wait();
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    NEON_INFO("Saved simple {} assets in {}ms", c_AssetCount, dt);
}

void AssetPackSample::LoadSimple()
{
    auto t0 = std::chrono::high_resolution_clock::now();

    auto t1 = std::chrono::high_resolution_clock::now();
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    NEON_INFO("Loaded simple {} assets in {}ms", c_AssetCount, dt);
}

//

///*
// We will build our asset graph like this:
//
//      0
//  1   2   3
// 4 5 6 7 8 9
//
//*/
//
// auto GenDepsArray()
//{
//     size_t i = 0;
//
//     std::array<AAsset::Handle, 10> Assets;
//     for (auto& Asset : Assets)
//     {
//         Asset = AAsset::Handle::FromString(StringUtils::Format("00000000-0000-0000-000{}-000000000000", i++));
//     }
//     return Assets;
// }
//
// void AssetPackSample::SaveDeps()
//{
//     auto Deps = GenDepsArray();
//
//     auto t0 = std::chrono::high_resolution_clock::now();
//
//     auto Manager = LoadManager();
//     auto Package = Manager->Mount(std::make_unique<AAsset::PackageDirectory>("Test/Directory1"));
//
//     std::array<Ptr<StringAndChildAsset>, 10> Assets;
//
//     // create assets from 4 to 9
//     for (size_t i = 4; i < Assets.size(); i++)
//     {
//         StringU8 Path = StringUtils::Format("Assets/Test{}.txt", i);
//
//         Assets[i] = Package->CreateAsset<StringAndChildAsset>(
//             { .Path = std::move(Path), .HandlerName = StringAndChildHandler::HandlerName },
//             StringU8(TextTest),
//             Deps[i]);
//     }
//
//     // create assets from 1 to 3
//     for (size_t i = 1; i < 4; i++)
//     {
//         StringU8 Path = StringUtils::Format("Assets/Test{}.txt", i);
//
//         std::array Chidren = { Assets[i * 2 + 2],
//                                Assets[i * 2 + 3] };
//
//         Assets[i] = Package->CreateAsset<StringAndChildAsset>(
//             { .Path = std::move(Path), .HandlerName = StringAndChildHandler::HandlerName },
//             StringU8(TextTest),
//             Deps[i],
//             Chidren);
//     }
//
//     // create asset 0
//     StringU8   Path    = StringUtils::Format("Assets/Test{}.txt", 0);
//     std::array Chidren = { Assets[1], Assets[2], Assets[3] };
//
//     Assets[0] = Package->CreateAsset<StringAndChildAsset>(
//         { .Path = std::move(Path), .HandlerName = StringAndChildHandler::HandlerName },
//         StringU8(TextTest),
//         Deps[0],
//         Chidren);
//
//     Manager->Flush(Package);
//
//     auto t1 = std::chrono::high_resolution_clock::now();
//     auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
//
//     NEON_INFO("Saved deps {} assets in {}ms", Assets.size(), dt);
// }
//
// void AssetPackSample::LoadDeps()
//{
//     auto Deps = GenDepsArray();
//
//     auto t0 = std::chrono::high_resolution_clock::now();
//
//     auto Manager = LoadManager();
//
//     auto Package = Manager->Mount(std::make_unique<AAsset::PackageDirectory>("Test/Directory1"));
//
//     auto Asset = Manager->Load(Package, Deps[0]).get();
//
//     auto t1 = std::chrono::high_resolution_clock::now();
//     auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
//
//     NEON_INFO("Loaded deps asset in {}ms", dt);
// }
