#include <Runtime/GameEngine.hpp>
#include <Resource/Manager.hpp>
#include <Resource/Packs/ZipPack.hpp>
#include <Resource/Types/TextFile.hpp>

#include <boost/uuid/uuid_generators.hpp>
#include <Log/Logger.hpp>

using namespace Neon;

class TestGameEngine : public DefaultGameEngine
{
public:
    TestGameEngine() :
        DefaultGameEngine(GetConfig())
    {
        WriteResources();
        LoadResources();
    }

    void WriteResources()
    {
        auto Manager = GetResourceManager();
        auto Pack    = Manager->NewPack<Asset::ZipAssetPack>("MainPack");

        const Asset::AssetHandle* Guids[]{
            &m_Data1,
            &m_Data2,
            &m_Data3
        };
        for (int i = 0; i < 3; i++)
        {
            auto Text = std::make_shared<Asset::TextFileAsset>(
                StringUtils::Format(L"Data{0}, this is some data {0}", i));
            Pack->Save(
                *Guids[i],
                Text);
        }

        Pack->Export("Data.np");
    }

    void LoadResources()
    {
        auto Manager = GetResourceManager();
        auto Pack    = Manager->LoadPack("SamePack", "Data.np");

        const Asset::AssetHandle* Guids[]{
            &m_Data1,
            &m_Data2,
            &m_Data3
        };
        for (int i = 0; i < 3; i++)
        {
            auto Text = std::dynamic_pointer_cast<Asset::TextFileAsset>(
                Pack->Load(*Guids[i]).lock());
            NEON_INFO(Text->AsUtf8());
        }
    }

private:
    [[nodiscard]] static Config::EngineConfig GetConfig()
    {
        Config::EngineConfig Config;
        Config.Window.Title      = L"Test Engine";
        Config.Window.Windowed   = true;
        Config.Window.Fullscreen = false;
        return Config;
    }

    [[nodiscard]] static Asset::AssetHandle MakeGuid(
        const char* Name)
    {
        return { boost::uuids::name_generator_latest(boost::uuids::ns::x500dn())(Name) };
    }

private:
    Asset::AssetHandle m_Data1{ MakeGuid("Data1") };
    Asset::AssetHandle m_Data2{ MakeGuid("Data2") };
    Asset::AssetHandle m_Data3{ MakeGuid("Data4") };
};

NEON_MAIN(Argc, Argv)
{
    Neon::Logger::SetLogTag("", Logger::LogSeverity::Trace);
    Neon::Logger::SetLogTag("Resource", Logger::LogSeverity::Info);
    Neon::Logger::SetLogTag("Window", Logger::LogSeverity::Trace);

    auto func = []
    {
        TestGameEngine Engine;
        return Engine.Run();
    };

    return func();
    // auto eng1 = std::async(func);

    // return eng1.get();
}