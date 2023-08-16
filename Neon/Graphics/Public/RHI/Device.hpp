#pragma once

#include <Core/Neon.hpp>
#include <RHI/DeviceConfig.hpp>

namespace Neon::Windowing
{
    class WindowApp;
}

namespace Neon::RHI
{
    class ISwapchain;
    struct SwapchainCreateDesc;

    class IRenderDevice
    {
        static inline DeviceCreateDesc::DescriptorSize s_DescriptorSize;

    public:
        /// <summary>
        /// Get the descriptor size of the render device
        /// </summary>
        [[nodiscard]] static const DeviceCreateDesc::DescriptorSize& GetDescriptorSize()
        {
            return s_DescriptorSize;
        }

        static void Create(
            Windowing::WindowApp*     Window,
            const DeviceCreateDesc&    DeviceDesc,
            const SwapchainCreateDesc& SwapchainDesc);

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