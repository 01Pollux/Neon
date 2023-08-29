#include <EnginePCH.hpp>
#include <Script/Handle.hpp>

#include <Private/Script/Internal/Engine.hpp>

namespace Neon::Scripting
{
    MonoObject* GCHandle::GetObject() const
    {
        return CS::ScriptContext::Get()->HandleMgr.GetObject(*this);
    }

    void GCHandle::Free()
    {
        CS::ScriptContext::Get()->HandleMgr.Free(*this);
    }

    bool GCHandle::IsAlive() const
    {
        return CS::ScriptContext::Get()->HandleMgr.IsAlive(*this);
    }
} // namespace Neon::Scripting