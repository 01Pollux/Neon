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

    Neon::Logger::Shutdown();
    return 0;
}