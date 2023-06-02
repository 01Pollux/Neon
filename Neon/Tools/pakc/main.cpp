#include <PakCPCH.hpp>
#include <Parser/JsonReader.hpp>

#include <Log/Logger.hpp>

#include <Resource/Runtime/Manager.hpp>
#include <Resource/Types/TextFile.hpp>
#include <boost/uuid/uuid_io.hpp>

int main(int Argc, char* Argv[])
{
    Neon::Logger::Initialize();
    Neon::Logger::SetLogTag("", Neon::Logger::LogSeverity::Info);

    if (Argc <= 1)
    {
        NEON_WARNING("Usage: pakc <file>...");
        return 0;
    }

    {
        PakC::JsonHandler Handler;
        for (int i = 1; i < Argc; i++)
        {
            Handler.Parse(Argv[i]);
        }
    }
    {
        Neon::Asset::RuntimeResourceManager Manager;

        auto Pack = Manager.LoadPack("MyPack", "samples/basic/hello_world.np");
        for (auto& Asset : Pack->GetAssets())
        {
            auto Resource = std::dynamic_pointer_cast<Neon::Asset::TextFileAsset>(Pack->Load(Asset).lock());
            NEON_INFO("Asset: {} -- Text: {}", boost::uuids::to_string(Asset), Resource->AsUtf8());
        }
    }

    Neon::Logger::Shutdown();
    return 0;
}