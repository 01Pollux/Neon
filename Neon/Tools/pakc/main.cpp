#include <PakCPCH.hpp>
#include <Runtime/EntryPoint.hpp>
#include <Parser/JsonReader.hpp>

#include <RHI/Device.hpp>
#include <Runtime/GameEngine.hpp>

#include <Log/Logger.hpp>

using namespace Neon;

NEON_MAIN(Argc, Argv)
{
    Logger::SetLogTag("", Logger::LogSeverity::Info);
    Logger::SetLogTag("Resource", Logger::LogSeverity::Info);
    Logger::SetLogTag("Graphics", Logger::LogSeverity::Info);
    Logger::SetLogTag("ShaderCompiler", Logger::LogSeverity::Info);

    if (Argc <= 1)
    {
        NEON_FATAL("Usage: pakc <file>...");
        return 0;
    }

    Config::EngineConfig Config{
        .Window = {
            .Title      = STR("PakC"),
            .Windowed   = true,
            .Fullscreen = false },
        .LoggerAssetUid = std::nullopt
    };

    Runtime::DefaultGameEngine Engine;
    Engine.Initialize(Config);

    PakC::JsonHandler Handler;
    for (int i = 1; i < Argc; i++)
    {
        Handler.Parse(StringUtils::Transform<StringU8>(String(Argv[i])));
    }

    return 0;
}