#include <Runtime/GameEngine.hpp>

int Neon::Main(
    int      Argc,
    wchar_t* Argv[])
{
    DefaultGameEngine Engine;
    return Engine.Run();
}
