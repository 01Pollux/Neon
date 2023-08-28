#include <EnginePCH.hpp>
#include <Script/Handle.hpp>

#include <Private/Script/HandleManager.hpp>

namespace Neon::Scripting
{
    MonoObject* GCHandle::GetObject() const
    {
        return HandleManager::Get()->GetObject(*this);
    }

    void GCHandle::Free()
    {
        HandleManager::Get()->Free(*this);
    }

    bool GCHandle::IsAlive() const
    {
        return HandleManager::Get()->IsAlive(*this);
    }
} // namespace Neon::Scripting