#include <PakCPCH.hpp>
#include <Parser/JsonReader.hpp>

#include <RHI/Device.hpp>
#include <Runtime/GameEngine.hpp>

#include <Log/Logger.hpp>

int main(int Argc, char* Argv[])
{
    Neon::Logger::Initialize();

    Neon::Logger::SetLogTag("", Neon::Logger::LogSeverity::Info);
    Neon::Logger::SetLogTag("Resource", Neon::Logger::LogSeverity::Info);
    Neon::Logger::SetLogTag("Graphics", Neon::Logger::LogSeverity::Info);
    Neon::Logger::SetLogTag("ShaderCompiler", Neon::Logger::LogSeverity::Info);

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