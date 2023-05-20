#pragma once

namespace Neon::RHI
{
    class IRenderDevice
    {
    public:
        static void CreateGlobal();
        static void DestroyGlobal();

        virtual ~IRenderDevice() = default;

        /// <summary>
        /// Gets the global render device.
        /// </summary>
        [[nodiscard]] static IRenderDevice* Get();
    };
} // namespace Neon::RHI