#pragma once

#include <Core/Neon.hpp>

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

        struct Instance
        {
            Instance()
            {
                CreateGlobal();
            }
            NEON_CLASS_NO_COPYMOVE(Instance);
            ~Instance()
            {
                DestroyGlobal();
            }
        };
    };
} // namespace Neon::RHI