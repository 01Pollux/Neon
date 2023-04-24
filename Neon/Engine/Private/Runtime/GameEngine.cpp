#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>
#include <Module/Core.hpp>

namespace Neon
{
    DefaultGameEngine::DefaultGameEngine()
    {
        ImportModule<Module::Core>();
    }

    DefaultGameEngine::~DefaultGameEngine()
    {
    }

    int DefaultGameEngine::Run()
    {
        return 0;
    }
} // namespace Neon
