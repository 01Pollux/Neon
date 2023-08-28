#include <EnginePCH.hpp>
#include <Private/Script/HandleManager.hpp>

#include <Mono/metadata/object.h>
#include <Mono/metadata/mono-gc.h>

#include <Log/Logger.hpp>

namespace Neon::Scripting
{
    void HandleManager::Shutdown()
    {
        if (m_StrongTrackedObjects.size())
        {
            NEON_ERROR_TAG("Script", "Memory leak detected, not all strong GCHandles were destroyed");

            for (auto Handle : m_StrongTrackedObjects | std::views::keys)
            {
                mono_gchandle_free(Handle);
            }
        }

        if (m_WeakTrackedObjects.size())
        {
            NEON_ERROR_TAG("Script", "Memory leak detected, not all weak GCHandles were destroyed");

            for (auto Handle : m_WeakTrackedObjects | std::views::keys)
            {
                mono_gchandle_free(Handle);
            }
        }

        Collect();
        WaitForPendingFinalizers();
    }

    GCHandle HandleManager::AddReference(
        MonoObject* managedObject,
        bool        WeakRef,
        bool        Pinned,
        bool        Track)
    {
        auto Handle = WeakRef ? mono_gchandle_new_weakref(managedObject, Pinned) : mono_gchandle_new(managedObject, Pinned);
        if (Track)
        {
            auto& TrackedObjects   = WeakRef ? m_WeakTrackedObjects : m_StrongTrackedObjects;
            TrackedObjects[Handle] = managedObject;
        }
        return Handle;
    }

    void HandleManager::Free(
        GCHandle Handle)
    {
#ifndef NEON_DIST
        NEON_ASSERT(mono_gchandle_get_target(Handle.Data), "Tried to release an object reference using an invalid handle!");
#endif
        mono_gchandle_free(Handle.Data);

        m_StrongTrackedObjects.erase(Handle.Data);
        m_WeakTrackedObjects.erase(Handle.Data);
    }

    MonoObject* HandleManager::GetObject(
        GCHandle Handle)
    {
        return mono_gchandle_get_target(Handle.Data);
    }

    bool HandleManager::IsAlive(
        GCHandle Handle)
    {
        MonoObject* Object = Handle.Data ? mono_gchandle_get_target(Handle.Data) : nullptr;
        return Object && mono_object_get_vtable(Object);
    }

    void HandleManager::Collect()
    {
        mono_gc_collect(mono_gc_max_generation());
    }

    void HandleManager::Collect(
        uint32_t Generation)
    {
        mono_gc_collect(int(Generation));
    }

    uint32_t HandleManager::MaxGeneration()
    {
        return uint32_t(mono_gc_max_generation());
    }

    void HandleManager::WaitForPendingFinalizers()
    {
        if (mono_gc_pending_finalizers())
        {
            mono_gc_finalize_notify();
            do
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            } while (mono_gc_pending_finalizers());
        }
    }
} // namespace Neon::Scripting