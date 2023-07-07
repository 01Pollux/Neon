#pragma once

#include <Utils/Singleton.hpp>

namespace Neon::RHI
{
    class ISwapchain;
    struct SwapchainCreateDesc;

    class IRenderDevice
    {
        friend class Utils::Singleton<IRenderDevice>;

    public:
        /// <summary>
        /// </summary>
        /// <param name="...Args"></param>
        static void Create(
            const SwapchainCreateDesc& Swapchain);

        /// <summary>
        /// Get the render device instance
        /// </summary>
        [[nodiscard]] static IRenderDevice* Get();

        /// <summary>
        /// Destroy the render device instance
        /// </summary>
        static void Destroy();

    public:
        IRenderDevice() = default;
        NEON_CLASS_NO_COPYMOVE(IRenderDevice);
        virtual ~IRenderDevice() = default;

        /// <summary>
        /// Get the swapchain instance of the render device
        /// </summary>
        [[nodiscard]] virtual RHI::ISwapchain* GetSwapchain() = 0;
    };
} // namespace Neon::RHI