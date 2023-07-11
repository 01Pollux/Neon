#pragma once

namespace Neon::RHI
{
    struct DeviceCreateDesc
    {
        struct DescriptorSize
        {
            /// <summary>
            /// A static descriptor heap is CPU only heap that is a descriptor heap that is persistent and can be used by multiple threads
            /// It is used for resources that are not frequently created and destroyed
            /// </summary>
            uint32_t Static_Resource = 65'536;
            uint32_t Static_Sampler  = 256;
            uint32_t Static_Rtv      = 4'096;
            uint32_t Static_Dsv      = 1'024;

            /// <summary>
            /// A staged descriptor heap is a CPU only heap that is a descriptor heap that persists for a single frame and can be used by multiple threads
            /// It is used for resources that are frequently created and destroyed
            /// </summary>
            uint32_t Staged_Resource = 32'768;
            uint32_t Staged_Sampler  = 128;
            uint32_t Staged_Rtv      = 256;
            uint32_t Staged_Dsv      = 64;

            /// <summary>
            /// A frame descriptor heap is a GPU-CPU heap that is a descriptor heap that persists for a single frame and can be used by multiple threads
            /// It is used for uploading resources to the GPU
            /// </summary>
            uint32_t Frame_Resource = 524'288;
            uint32_t Frame_Sampler  = 512;
        };

        DescriptorSize Descriptors;

        /// <summary>
        /// Enable the debug layer for the render device (non-production)
        /// </summary>
        bool EnableDebugLayer : 1 = true;

        /// <summary>
        /// Enable the device removed extended data (DRED) layer for the render device (non-production)
        /// </summary>
        bool EnableDredLayer : 1 = true;

        /// <summary>
        /// Enable the gpu debugger for the render device (non-production)
        /// </summary>
        bool EnableGPUDebugger : 1 = true;

        /// <summary>
        /// Enable the gpu based validation for the render device (non-production)
        /// </summary>
        bool EnableGpuBasedValidation : 1 = false;
    };
} // namespace Neon::RHI