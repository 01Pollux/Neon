#pragma once

#include <RHI/Resource/Common.hpp>

namespace Neon::RHI
{
    struct SwapchainCreateDesc
    {
        struct
        {
            /// <summary>
            /// The numerator of the refresh rate.
            /// </summary>
            uint16_t Numerator = 0;

            /// <summary>
            /// The denominator of the refresh rate.
            /// </summary>
            uint16_t Denominator = 0;
        } RefreshRate;

        struct
        {
            /// <summary>
            /// The number of multisamples per pixel.
            /// </summary>
            uint16_t Count = 1;

            /// <summary>
            /// The image quality level. The higher the quality, the lower the performance.
            /// </summary>
            uint16_t Quality = 0;
        } Sample;

        /// <summary>
        /// The number of frames in flight.
        /// </summary>
        uint32_t FramesInFlight = 3;

        /// <summary>
        /// the format of the backbuffer.
        /// </summary>
        EResourceFormat BackbufferFormat = EResourceFormat::R8G8B8A8_UNorm;

        /// <summary>
        /// Enable vertical sync.
        /// </summary>
        bool VSync : 1 = true;
    };
} // namespace Neon::RHI