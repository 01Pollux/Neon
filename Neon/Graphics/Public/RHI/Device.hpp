#pragma once

#include <Utils/Singleton.hpp>

namespace Neon::RHI
{
    class ISwapchain;
    struct SwapchainCreateDesc;

    class IRenderDevice : public Utils::Singleton<IRenderDevice>
    {
        friend class Utils::Singleton<IRenderDevice>;

    public:
        IRenderDevice() = default;
        NEON_CLASS_NO_COPYMOVE(IRenderDevice);
        virtual ~IRenderDevice() = default;

        /// <summary>
        /// Get the swapchain instance of the render device
        /// </summary>
        [[nodiscard]] virtual RHI::ISwapchain* GetSwapchain() = 0;

    private:
        /// <summary>
        /// Create a render device instance
        /// </summary>
        [[nodiscard]] static IRenderDevice* SConstruct(
            const SwapchainCreateDesc& Swapchain);
    };
} // namespace Neon::RHI