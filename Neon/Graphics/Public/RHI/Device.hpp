#pragma once

namespace Neon::RHI
{
    class IResourceStateManager;

    class IRenderDevice
    {
    public:
        static IRenderDevice* CreateGlobal();
        static void           DestroyGlobal();

        virtual ~IRenderDevice() = default;

        /// <summary>
        /// Gets the global render device.
        /// </summary>
        [[nodiscard]] static IRenderDevice* Get();

        /// <summary>
        /// Get the resource state manager.
        /// </summary>
        [[nodiscard]] virtual IResourceStateManager* GetStateManager() = 0;
    };
} // namespace Neon::RHI