#pragma once

#include <flecs/flecs.h>

namespace Neon
{
    extern int Main(
        int      Argc,
        wchar_t* Argv[]);

    class DefaultGameEngine
    {
    public:
        DefaultGameEngine();

        DefaultGameEngine(const DefaultGameEngine&)            = delete;
        DefaultGameEngine& operator=(const DefaultGameEngine&) = delete;

        DefaultGameEngine(DefaultGameEngine&&)             = delete;
        DefaultGameEngine&& operator=(DefaultGameEngine&&) = delete;

        ~DefaultGameEngine();

        /// <summary>
        /// Run the engine and return when the engine is closed or an error occurs.
        /// </summary>
        int Run();

    protected:
        template<typename _Ty>
        void ImportModule()
        {
            m_World.import <_Ty>();
        }

    private:
        flecs::world m_World;
    };
} // namespace Neon

#if NEON_DIST

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define NEON_MAIN()                         \
    int WINAPI wWinMain(                    \
        HINSTANCE hInstance,                \
        HINSTANCE hPrevInstance,            \
        LPTSTR    lpCmdLine,                \
        int       nShowCmd)                 \
    {                                       \
        return Neon::Main(__argc, __wargv); \
    }                                       \
    int Neon::Main(                         \
        int      Argc,                      \
        wchar_t* Argv[])

#else

#define NEON_MAIN()                    \
    int main(                          \
        int       argc,                \
        wchar_t** argv)                \
    {                                  \
        return Neon::Main(argc, argv); \
    }                                  \
    int Neon::Main(                    \
        int      Argc,                 \
        wchar_t* Argv[])

#endif
