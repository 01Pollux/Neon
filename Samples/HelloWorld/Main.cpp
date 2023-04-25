#include <Runtime/GameEngine.hpp>
#include <Log/Logger.hpp>

NEON_MAIN(Argc, Argv)
{
    DefaultGameEngine Engine;
    return Engine.Run();
}
