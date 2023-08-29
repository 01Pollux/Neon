#include <EnginePCH.hpp>
#include <Private/Script/Calls.hpp>
#include <Private/Script/Internal/Engine.hpp>

namespace Neon::Scripting::CS::InternalCalls
{
#define REGISTER_INTERNAL_CALL(Name, Callback) mono_add_internal_call("Neon."##Name, std::bit_cast<const void*>(Callback))

    void Register()
    {
        // REGISTER_INTERNAL_CALL("Engine::GetEngine", GetEngine);
    }
} // namespace Neon::Scripting::CS::InternalCalls