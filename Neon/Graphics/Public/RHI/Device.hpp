#pragma once

namespace Neon::RHI
{
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
    };
} // namespace Neon::RHI