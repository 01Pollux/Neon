#include <NModulePCH.hpp>
#include <Runtime/GameEngine.hpp>

namespace Neon
{
    DefaultGameEngine::DefaultGameEngine()
    {
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
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine,
    int       nShowCmd)
{
    return Neon::Main(__argc, __wargv);
}

#else

int main(
    int       argc,
    wchar_t** argv)
{
    return Neon::Main(argc, argv);
}

#endif
