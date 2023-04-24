#include <Runtime/GameEngine.hpp>

int Neon::Main(
    int      argc,
    wchar_t* argv[])
{
    DefaultGameEngine Engine;
    return Engine.Run();
}
