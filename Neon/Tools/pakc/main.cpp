#include <PakCPCH.hpp>
#include <Parser/JsonReader.hpp>

#include <RHI/Device.hpp>
#include <Runtime/GameEngine.hpp>

#include <Log/Logger.hpp>

#include <Resource/Runtime/Manager.hpp>
#include <Resource/Types/TextFile.hpp>
#include <boost/uuid/uuid_io.hpp>

int main(int Argc, char* Argv[])
{
    Neon::Logger::Initialize();

    if (Argc <= 1)
    {
        NEON_WARNING("Usage: pakc <file>...");
        return 0;
    }

    Neon::RHI::IRenderDevice::CreateGlobal();

    {
        PakC::JsonHandler Handler;
        for (int i = 1; i < Argc; i++)
        {
            Handler.Parse(Argv[i]);
        }
    }

    Neon::RHI::IRenderDevice::DestroyGlobal();

    Neon::Logger::Shutdown();
    return 0;
}