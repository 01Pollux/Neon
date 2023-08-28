#pragma once

typedef struct _MonoObject MonoObject;

namespace Neon::Scripting
{
    struct GCHandle
    {
        uint32_t Data;

        GCHandle(
            uint32_t Data = 0) :
            Data(Data)
        {
        }

        /// <summary>
        /// Gets the object.
        /// </summary>
        [[nodiscard]] MonoObject* GetObject() const;

        /// <summary>
        /// Frees the handle.
        /// </summary>
        void Free();

        /// <summary>
        /// Checks if the handle is alive.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] bool IsAlive() const;

        /// <summary>
        /// Checks if the handle is alive.
        /// </summary>
        [[nodiscard]] operator bool() const
        {
            return IsAlive();
        }
    };

    using UGCHandle = std::unique_ptr<GCHandle, decltype([](GCHandle* Handle)
                                                         { Handle->Free(); })>;
} // namespace Neon::Scripting