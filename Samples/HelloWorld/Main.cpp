#include <Runtime/EntryPoint.hpp>

int main(int argc, wchar_t** argv)
{
    Neon::Logger::Initialize();
    int Ret = Neon::Runtime::Main(argc, argv);
    Neon::Logger::Shutdown();
    return Ret;
}
int Neon::Runtime::Main(int Argc, wchar_t* Argv[])
{
    Config::EngineConfig Config{
        .Window = {
            .Title      = STR("Test Engine"),
            .Windowed   = true,
            .Fullscreen = false }
    };
    return RunEngine<Runtime::DefaultGameEngine>(Config);
}