#pragma once

#include <RHI/DeviceConfig.hpp>
#include <RHI/SwapchainConfig.hpp>

namespace Neon::Config
{
    struct RendererConfig
    {
        /// <summary>
        /// The device creation descriptor.
        /// </summary>
        RHI::DeviceCreateDesc Device;

        /// <summary>
        /// The swapchain creation descriptor.
        /// </summary>
        RHI::SwapchainCreateDesc Swapchain;
    };
} // namespace Neon::Config