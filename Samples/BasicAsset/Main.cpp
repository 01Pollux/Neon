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

#include <Asset/Manager.hpp>
#include <Asset/Packages/Directory.hpp>

//

#include <Asset/Asset.hpp>
#include <Asset/Handler.hpp>

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
    Asio::CoLazy<Ptr<AAsset::IAsset>> Load(
        IO::InArchive2&               Archive,
        const AAsset::Handle&         Handle,
        AAsset::AssetDependencyGraph& Graph) override
    {
        AAsset::Handle ChildHandle;

        StringU8 Text;
        Archive >> Text;
        Archive >> ChildHandle;

        return std::make_shared<StringAndChildAsset>(std::move(Text), co_await Graph.Requires(Handle, ChildHandle));
    }
};

UPtr<AAsset::Manager> AssetPackSample::LoadManager()
{
    auto Manager = std::make_unique<AAsset::Manager>();
    Manager->RegisterHandler<StringAndChildHandler>("StringAndChild");

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

    auto Handle0 = AAsset::Handle::FromString("00000000-0000-0000-0000-000000000000");

    auto Asset = std::make_unique<StringAndChildAsset>(TextTest, Handle0);
}

void AssetPackSample::LoadSimple()
{
    auto Manager = LoadManager();
    auto Package = Manager->Mount(std::make_unique<AAsset::PackageDirectory>("Test/Directory1"));

    auto Handle0 = AAsset::Handle::FromString("00000000-0000-0000-0000-000000000000");

    auto Asset = std::make_unique<StringAndChildAsset>(TextTest, Handle0);
}
