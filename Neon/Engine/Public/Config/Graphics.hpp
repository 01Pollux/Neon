#pragma once

#include <Math/Size2.hpp>
#include <Core/String.hpp>

namespace Neon::Config
{
    struct GraphicsConfig
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
    };
} // namespace Neon::Config