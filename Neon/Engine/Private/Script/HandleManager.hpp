#pragma once

#include <Script/Handle.hpp>
#include <cstdint>

namespace Neon::Scripting
{
    class HandleManager
    {
    public:
        [[nodiscard]] static HandleManager* Get();

        /// <summary>
        /// Shuts down the handle manager.
        /// </summary>
        void Shutdown();

        /// <summary>
        /// Adds a reference to the specified managed object.
        /// </summary>
        GCHandle AddReference(
            MonoObject* managedObject,
            bool        WeakRef = false,
            bool        Pinned  = false,
            bool        Track   = true);

        /// <summary>
        /// Frees the handle.
        /// </summary>
        void Free(
            GCHandle Handle);

        /// <summary>
        /// Get the object from the handle.
        /// </summary>
        [[nodiscard]] MonoObject* GetObject(
            GCHandle Handle);

        /// <summary>
        /// Checks if the handle is alive.
        /// </summary>
        [[nodiscard]] bool IsAlive(
            GCHandle Handle);

        /// <summary>
        /// Collects all garbage.
        /// </summary>
        void Collect();

        /// <summary>
        /// Collects all garbage for the specified generation.
        /// </summary>
        void Collect(
            uint32_t Generation);

        /// <summary>
        /// Gets the current generation.
        /// </summary>
        [[nodiscard]] uint32_t MaxGeneration();

        /// <summary>
        /// Wait for all pending finalizers to finish.
        /// </summary>
        void WaitForPendingFinalizers();

    private:
        std::unordered_map<uint32_t, MonoObject*> m_WeakTrackedObjects;
        std::unordered_map<uint32_t, MonoObject*> m_StrongTrackedObjects;
    };
} // namespace Neon::Scripting