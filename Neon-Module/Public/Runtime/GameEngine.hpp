#pragma once

namespace Neon
{
    extern int Main(
        int      argc,
        wchar_t* argv[]);

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
    };
} // namespace Neon