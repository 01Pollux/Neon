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

        SaveSimple();
        LoadSimple();
    }

private:
    void SaveSimple();
    void LoadSimple();

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
        StringU8                   Text,
        const AAsset::Handle&      Handle = AAsset::Handle::Random(),
        const Ptr<AAsset::IAsset>& Child  = nullptr) :
        IAsset(Handle),
        m_Text(std::move(Text))
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
    StringU8            m_Text;
    Ptr<AAsset::IAsset> m_Child;
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
        AAsset::Handle ChildHandle;

        StringU8 Text;
        Archive >> Text;
        Archive >> ChildHandle;

        co_return std::make_shared<StringAndChildAsset>(std::move(Text), Handle, co_await Graph.Requires(Handle, ChildHandle));
    }

    void Save(
        IO::OutArchive2&           Archive,
        const Ptr<AAsset::IAsset>& Asset) override
    {
        auto ThisAsset = static_cast<const StringAndChildAsset*>(Asset.get());

        Archive << ThisAsset->GetText();
        Archive << (ThisAsset->m_Child ? ThisAsset->m_Child->GetGuid() : AAsset::Handle::Null);
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

void AssetPackSample::SaveSimple()
{
    auto Manager = LoadManager();
    auto Package = Manager->Mount(std::make_unique<AAsset::PackageDirectory>("Test/Directory1"));

    for (size_t i = 0; i < 10; i++)
    {
        StringU8 Path  = StringUtils::Format("Assets/Test{}.txt", i);
        auto     Asset = Package->CreateAsset<StringAndChildAsset>(
            { .Path = std::move(Path), .HandlerName = StringAndChildHandler::HandlerName },
            StringU8(TextTest));
    }

    Manager->Flush(Package);
}

void AssetPackSample::LoadSimple()
{
    auto Manager = LoadManager();
    auto Package = Manager->Mount(std::make_unique<AAsset::PackageDirectory>("Test/Directory1"));

    std::vector<std::future<Ptr<AAsset::IAsset>>> LoadingAssets;

    for (auto& Asset : Manager->GetPackageAssets(Package))
    {
        LoadingAssets.emplace_back(Manager->Load(Package, Asset));
    }

    auto LoadedAssets = LoadingAssets |
                        views::transform([](auto& Future)
                                         { return Future.get(); }) |
                        ranges::to<std::vector>();

    for (auto& Asset : LoadedAssets)
    {
        auto ThisAsset = dynamic_cast<const StringAndChildAsset*>(Asset.get());
        NEON_ASSERT(ThisAsset);
        NEON_ASSERT(ThisAsset->GetText() == TextTest);
    }

    /*  auto Manager = LoadManager();
      auto Package = Manager->Mount(std::make_unique<AAsset::PackageDirectory>("Test/Directory1"));

      auto Handle0 = AAsset::Handle::FromString("00000000-0000-0000-0000-000000000000");

      auto Asset = std::make_unique<StringAndChildAsset>(TextTest, Handle0);*/
}
