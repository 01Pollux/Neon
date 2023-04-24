#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>

namespace Neon
{
    DefaultGameEngine::DefaultGameEngine()
    {
        //ImportModule<Module::Core>();
    }

    DefaultGameEngine::~DefaultGameEngine()
    {
    }

    int DefaultGameEngine::Run()
    {
        return 0;
    }
} // namespace Neon

#if NEON_DIST

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define NEON_ENTRTY_POINT()
int WINAPI wWinMain(
    HINSTANCE,
    HINSTANCE,
    LPTSTR,
    int)
{
    return Neon::Main(__argc, __wargv);
}

#else

int main(
    int       Argc,
    wchar_t** Argv)
{
    return Neon::Main(Argc, Argv);
}

#endif
