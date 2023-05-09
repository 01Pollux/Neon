#pragma once

#include <Math/Size2.hpp>

namespace Neon::Windowing
{
    class IWindowApp;
}

namespace Neon::RHI
{
    class ISwapchain
    {
    public:
        struct InitDesc
        {
            Windowing::IWindowApp* Window;
            struct
            {
                uint16_t Numerator   = 60;
                uint16_t Denominator = 1;
            } RefreshRate;
            struct
            {
                uint16_t Count   = 1;
                uint16_t Quality = 0;
            } Sample;
            uint32_t FramesInFlight = 3;
            // TODO: Add more swapchain settings. such as format, vsync, etc.
        };

        [[nodiscard]] static ISwapchain* Create(
            const InitDesc& Desc);

        virtual ~ISwapchain() = default;

        /// <summary>
        /// Prepare frame for rendering.
        /// </summary>
        virtual void PrepareFrame() = 0;

        /// <summary>
        /// Render frame and present it to the swapchain.
        /// </summary>
        virtual void Present() = 0;

        /// <summary>
        /// Set swapchain's size.
        /// </summary>
        virtual void Resize(
            const Size2I& Size) = 0;
    };
} // namespace Neon::RHI