#include <Runtime/EntryPoint.hpp>
#include <Runtime/Types/WorldRuntime.hpp>

//

#include <Asset/Manager.hpp>
#include <Asset/Packages/Directory.hpp>

#include <Asset/Asset.hpp>
#include <Asset/Handler.hpp>

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

        // SaveSimple();
        // LoadSimple();

        // SaveDeps();
        LoadDeps();
    }

private:
    void SaveSimple();
    void LoadSimple();

    void SaveDeps();
    void LoadDeps();

private:
    [[nodiscard]] UPtr<AAsset::Manager> LoadManager();
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

    Asio::CoLazy<Ptr<AAsset::IAsset>> Load(
        IO::InArchive2&               Archive,
        const AAsset::Handle&         Handle,
        AAsset::AssetDependencyGraph& Graph) override
    {
        StringU8 Text;
        Archive >> Text;

        size_t ChildCount = 0;
        Archive >> ChildCount;

        std::vector<Ptr<StringAndChildAsset>> Children;
        Children.reserve(ChildCount);

        AAsset::Handle ChildHandle;
        for (size_t i = 0; i < ChildCount; i++)
        {
            Archive >> ChildHandle;
            Children.emplace_back(std::dynamic_pointer_cast<StringAndChildAsset>(co_await Graph.Requires(Handle, ChildHandle)));
        }

        co_return std::make_shared<StringAndChildAsset>(std::move(Text), Handle, Children);
    }

    void Save(
        IO::OutArchive2&           Archive,
        const Ptr<AAsset::IAsset>& Asset) override
    {
        auto ThisAsset = static_cast<const StringAndChildAsset*>(Asset.get());

        Archive << ThisAsset->GetText();
        Archive << ThisAsset->m_Children.size();
        for (auto& Child : ThisAsset->m_Children)
        {
            Archive << Child->GetGuid();
        }
    }
};

UPtr<AAsset::Manager> AssetPackSample::LoadManager()
{
    auto Manager = std::make_unique<AAsset::Manager>();
    Manager->RegisterHandler<StringAndChildHandler>();
    return Manager;
}

//

static const char* TextTest = R"(
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nullam euismod, nisl eget ultricies ultrices, nunc nisl ultricies nunc, quis ultric
ies nisl nisl eget nisl. Nullam euismod, nisl eget ultricies ultrices, nunc nisl ultricies nunc, quis ultricies nisl nisl eget nisl. Nullam eu
ismod, nisl eget ultricies ultrices, nunc nisl ultricies nunc, quis ultricies nisl nisl eget nisl. Nullam euismod, nisl eget ultricies ultric
)";

static constexpr uint32_t c_AssetCount = 100;

void AssetPackSample::SaveSimple()
{
    auto t0 = std::chrono::high_resolution_clock::now();

    auto Manager = LoadManager();
    auto Package = Manager->Mount(std::make_unique<AAsset::PackageDirectory>("Test/Directory1"));

    for (size_t i = 0; i < c_AssetCount; i++)
    {
        StringU8 Path  = StringUtils::Format("Assets/Test{}.txt", i);
        auto     Asset = Package->CreateAsset<StringAndChildAsset>(
            { .Path = std::move(Path), .HandlerName = StringAndChildHandler::HandlerName },
            StringU8(TextTest));
    }

    Manager->Flush(Package);

    auto t1 = std::chrono::high_resolution_clock::now();
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    NEON_INFO("Saved simple {} assets in {}ms", c_AssetCount, dt);
}

void AssetPackSample::LoadSimple()
{
    auto t0 = std::chrono::high_resolution_clock::now();

    auto Manager = LoadManager();
    auto Package = Manager->Mount(std::make_unique<AAsset::PackageDirectory>("Test/Directory1"));

    std::vector<std::future<Ref<AAsset::IAsset>>> LoadingAssets;

    for (auto& Asset : Manager->GetPackageAssets(Package))
    {
        LoadingAssets.emplace_back(Manager->Load(Package, Asset));
    }

    auto LoadedAssets = LoadingAssets |
                        views::transform([](auto& Future)
                                         { return Future.get().lock(); }) |
                        ranges::to<std::vector>();

    for (auto& Asset : LoadedAssets)
    {
        auto ThisAsset = dynamic_cast<const StringAndChildAsset*>(Asset.get());
        NEON_ASSERT(ThisAsset);
        NEON_ASSERT(ThisAsset->GetText() == TextTest);
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    NEON_INFO("Loaded simple {} assets in {}ms", LoadedAssets.size(), dt);
}

//

/*
We will build our asset graph like this:

     0
 1   2   3
4 5 6 7 8 9

*/

auto GenDepsArray()
{
    size_t i = 0;

    std::array<AAsset::Handle, 10> Assets;
    for (auto& Asset : Assets)
    {
        Asset = AAsset::Handle::FromString(StringUtils::Format("00000000-0000-0000-000{}-000000000000", i++));
    }
    return Assets;
}

void AssetPackSample::SaveDeps()
{
    auto Deps = GenDepsArray();

    auto t0 = std::chrono::high_resolution_clock::now();

    auto Manager = LoadManager();
    auto Package = Manager->Mount(std::make_unique<AAsset::PackageDirectory>("Test/Directory1"));

    std::array<Ptr<StringAndChildAsset>, 10> Assets;

    // create assets from 4 to 9
    for (size_t i = 4; i < Assets.size(); i++)
    {
        StringU8 Path = StringUtils::Format("Assets/Test{}.txt", i);

        Assets[i] = Package->CreateAsset<StringAndChildAsset>(
            { .Path = std::move(Path), .HandlerName = StringAndChildHandler::HandlerName },
            StringU8(TextTest),
            Deps[i]);
    }

    // create assets from 1 to 3
    for (size_t i = 1; i < 4; i++)
    {
        StringU8 Path = StringUtils::Format("Assets/Test{}.txt", i);

        std::array Chidren = { Assets[i * 2 + 2],
                               Assets[i * 2 + 3] };

        Assets[i] = Package->CreateAsset<StringAndChildAsset>(
            { .Path = std::move(Path), .HandlerName = StringAndChildHandler::HandlerName },
            StringU8(TextTest),
            Deps[i],
            Chidren);
    }

    // create asset 0
    StringU8   Path    = StringUtils::Format("Assets/Test{}.txt", 0);
    std::array Chidren = { Assets[1], Assets[2], Assets[3] };

    Assets[0] = Package->CreateAsset<StringAndChildAsset>(
        { .Path = std::move(Path), .HandlerName = StringAndChildHandler::HandlerName },
        StringU8(TextTest),
        Deps[0],
        Chidren);

    Manager->Flush(Package);

    auto t1 = std::chrono::high_resolution_clock::now();
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    NEON_INFO("Saved deps {} assets in {}ms", Assets.size(), dt);
}

void AssetPackSample::LoadDeps()
{
    auto Deps = GenDepsArray();

    auto t0 = std::chrono::high_resolution_clock::now();

    auto Manager = LoadManager();

    auto Package = Manager->Mount(std::make_unique<AAsset::PackageDirectory>("Test/Directory1"));

    auto Asset = Manager->Load(Package, Deps[0]).get();

    auto t1 = std::chrono::high_resolution_clock::now();
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    NEON_INFO("Loaded deps asset in {}ms", dt);
}
