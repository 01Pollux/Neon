#include <Runtime/EntryPoint.hpp>
#include <Runtime/Types/WorldRuntime.hpp>

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

        RegisterManager();

        // SaveSimple();
        LoadSimple();

        // SaveDeps();
        // LoadDeps();
    }

private:
    void RegisterManager();

    void SaveSimple();
    void LoadSimple();

    void SaveDeps();
    void LoadDeps();
};

NEON_MAIN(Argc, Argv)
{
    Config::EngineConfig Config{
        .Window = {
            .Title          = STR("Test Engine"),
            .Windowed       = true,
            .Fullscreen     = false,
            .InitialVisible = false },
    };
    return RunEngine<AssetPackSample>(Config);
}

//

static constexpr uint32_t c_AssetCount = 100;
static const char*        TextTest =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nullam euismod, nisl eget ultricies ultrices, nunc nisl ultricies nunc, quis ultric\n"
    "ies nisl nisl eget nisl. Nullam euismod, nisl eget ultricies ultrices, nunc nisl ultricies nunc, quis ultricies nisl nisl eget nisl. Nullam eu\n"
    "ismod, nisl eget ultricies ultrices, nunc nisl ultricies nunc, quis ultricies nisl nisl eget nisl. Nullam euismod, nisl eget ultricies ultric";

//

class StringAndChildAsset : public Asset::IAsset
{
    friend class StringAndChildHandler;

public:
    StringAndChildAsset(
        StringU8                            Text,
        StringU8                            Path,
        const Asset::Handle&                Handle   = Asset::Handle::Random(),
        std::span<Ptr<StringAndChildAsset>> Children = {}) :
        IAsset(Handle, std::move(Path)),
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

    void Validate()
    {
        for (auto& Child : m_Children)
        {
            if (Child == nullptr)
            {
                continue;
            }

            Child->Validate();
        }

        NEON_ASSERT(GetText() == TextTest, "Asset text mismatch");
    }

    /// <summary>
    /// Used for ::LoadDeps()
    /// </summary>
    void ValidateChildrenGraph(
        size_t Count = 3)
    {
        if (Count > 0)
        {
            NEON_ASSERT(m_Children.size() == Count, "Invalid children count");
            for (auto& Child : m_Children)
            {
                if (Child == nullptr)
                {
                    continue;
                }

                Child->ValidateChildrenGraph(Count == 2 ? 0 : 2);
            }
        }
    }

private:
    StringU8                              m_Text;
    std::vector<Ptr<StringAndChildAsset>> m_Children;
};

//

class StringAndChildHandler : public Asset::IAssetHandler
{
public:
    static constexpr const char* HandlerName = "StringAndChild";

    bool CanHandle(
        const Ptr<Asset::IAsset>& Asset) override
    {
        return std::dynamic_pointer_cast<StringAndChildAsset>(Asset) != nullptr;
    }

    Ptr<Asset::IAsset> Load(
        std::ifstream&                 Stream,
        const Asset::DependencyReader& DepReader,
        const Asset::Handle&           AssetGuid,
        StringU8                       Path,
        const Asset::AssetMetaData&    LoaderData) override
    {
        boost::archive::text_iarchive Archive(Stream, boost::archive::no_header | boost::archive::no_tracking);

        StringU8 Text;
        Archive >> Text;

        auto ChildrenAssets = DepReader.ReadMany<StringAndChildAsset>(Archive);
        return std::make_shared<StringAndChildAsset>(std::move(Text), std::move(Path), AssetGuid, ChildrenAssets);
    }

    void Save(
        std::fstream&             Stream,
        Asset::DependencyWriter&  DepWriter,
        const Ptr<Asset::IAsset>& Asset,
        Asset::AssetMetaData&     LoaderData) override
    {
        boost::archive::text_oarchive Archive(Stream, boost::archive::no_header | boost::archive::no_tracking);

        auto StringAndChild = static_cast<StringAndChildAsset*>(Asset.get());
        Archive << StringAndChild->GetText();

        DepWriter.WriteMany(Archive, StringAndChild->m_Children);
    }
};

//

void AssetPackSample::RegisterManager()
{
    Asset::Storage::RegisterHandler<StringAndChildHandler>();

    //

    Asset::Storage::Mount(std::make_unique<Asset::DirectoryAssetPackage>("Test"));
}

//

void AssetPackSample::SaveSimple()
{
    std::vector<std::future<void>> Tasks;
    Tasks.reserve(c_AssetCount);

    auto t0 = std::chrono::high_resolution_clock::now();

    for (uint32_t i = 1; i <= c_AssetCount; ++i)
    {
        StringU8 Path = StringUtils::Format("File/{}.txt", i);

        auto AssetGuid = Asset::Handle::FromString(StringUtils::Format("00000000-0000-{:0>4}-0000-000000000000", i));
        auto Asset     = std::make_shared<StringAndChildAsset>(TextTest, std::move(Path), AssetGuid);

        Tasks.emplace_back(Asset::Storage::SaveAsset(
            { .Asset = std::move(Asset) }));
    }

    for (auto& Task : Tasks)
    {
        Task.get();
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    NEON_INFO("Saved simple {} assets in {}ms", c_AssetCount, dt);
    Asset::Storage::ExportAll();
}

//

void AssetPackSample::LoadSimple()
{
    std::vector<std::future<Ptr<Asset::IAsset>>> Tasks;
    Tasks.reserve(c_AssetCount);

    auto t0 = std::chrono::high_resolution_clock::now();

    for (uint32_t i = 1; i <= c_AssetCount; ++i)
    {
        auto AssetGuid = Asset::Handle::FromString(StringUtils::Format("00000000-0000-{:0>4}-0000-000000000000", i));
        Tasks.emplace_back(Asset::Manager::Load(AssetGuid));
    }

    for (auto& Task : Tasks)
    {
        auto Asset = Task.get();
        NEON_ASSERT(Asset != nullptr, "Failed to load asset");
        std::dynamic_pointer_cast<StringAndChildAsset>(Asset)->Validate();
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    NEON_INFO("Loaded simple {} assets in {}ms", c_AssetCount, dt);
}

//

auto GenDepsArray()
{
    std::array<Asset::Handle, 10> Handles;

    size_t i = 1;
    for (auto& Handle : Handles)
    {
        Handle = Asset::Handle::FromString(StringUtils::Format("00000000-0000-{:0>4}-0000-000000000000", i++));
    }

    return Handles;
}

//

/*
 We will build our asset graph like this:

      0
  1   2   3
 4 5 6 7 8 9
*/
void AssetPackSample::SaveDeps()
{
    auto AssetGuids = GenDepsArray();

    std::array<Ptr<StringAndChildAsset>, 10> Assets;

    auto t0 = std::chrono::high_resolution_clock::now();

    // create assets from 4 to 9
    for (size_t i = 4; i < Assets.size(); i++)
    {
        StringU8 Path = StringUtils::Format("File/{}.txt", i);

        Assets[i] = std::make_shared<StringAndChildAsset>(TextTest, std::move(Path), AssetGuids[i]);

        // Its not required to save asset right now, because we will save asset 0 later
        Asset::Storage::SaveAsset(
            { .Asset = Assets[i] })
            .get();
    }

    // create assets from 1 to 3
    for (size_t i = 1; i < 4; i++)
    {
        StringU8 Path = StringUtils::Format("File/{}.txt", i);

        std::array Children = { Assets[i * 2 + 2],
                                Assets[i * 2 + 3] };

        Assets[i] = std::make_shared<StringAndChildAsset>(TextTest, std::move(Path), AssetGuids[i], Children);

        // Its not required to save asset right now, because we will save asset 0 later
        Asset::Storage::SaveAsset(
            { .Asset = Assets[i] })
            .get();
    }

    // create asset 0
    StringU8   Path     = StringUtils::Format("File/{}.txt", 0);
    std::array Children = { Assets[1], Assets[2], Assets[3] };

    Assets[0] = std::make_shared<StringAndChildAsset>(TextTest, std::move(Path), AssetGuids[0], Children);
    Asset::Storage::SaveAsset(
        { .Asset = Assets[0] })
        .get();

    auto t1 = std::chrono::high_resolution_clock::now();
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    NEON_INFO("Saved deps {} assets in {}ms", Assets.size(), dt);

    Asset::Storage::ExportAll();
}

//

void AssetPackSample::LoadDeps()
{
    auto Deps = GenDepsArray();

    auto t0 = std::chrono::high_resolution_clock::now();

    auto Asset = Asset::Manager::Load(Deps[0]).get();
    NEON_ASSERT(Asset != nullptr, "Failed to load asset");
    std::dynamic_pointer_cast<StringAndChildAsset>(Asset)->Validate();
    std::dynamic_pointer_cast<StringAndChildAsset>(Asset)->ValidateChildrenGraph();

    auto t1 = std::chrono::high_resolution_clock::now();
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    NEON_INFO("Loaded deps asset in {}ms", dt);
}
