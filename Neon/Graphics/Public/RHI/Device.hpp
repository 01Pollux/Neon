#pragma once

#include <Utils/Singleton.hpp>

namespace Neon::RHI
{
    class ISwapchain;
    struct SwapchainCreateDesc;

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
    };

    class IRenderDevice
    {
        friend class Utils::Singleton<IRenderDevice>;

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