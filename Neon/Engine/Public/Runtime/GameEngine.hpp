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