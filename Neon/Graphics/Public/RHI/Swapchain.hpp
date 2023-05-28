#pragma once

#include <Math/Size2.hpp>
#include <RHI/Commands/Commands.hpp>
#include <RHI/Resource/Descriptor.hpp>

namespace Neon::Windowing
{
    class IWindowApp;
}

namespace Neon::RHI
{
    class IGpuResource;
    class IResourceStateManager;
    class ICommandQueue;
    class IFence;

    class ISwapchain
    {
    public:
        struct InitDesc
        {
            Windowing::IWindowApp* Window;
            struct
            {
                uint16_t Numerator   = 0;
                uint16_t Denominator = 0;
            } RefreshRate;
            struct
            {
                uint16_t Count   = 1;
                uint16_t Quality = 0;
            } Sample;
            uint32_t        FramesInFlight   = 3;
            EResourceFormat BackbufferFormat = EResourceFormat::R8G8B8A8_UNorm;

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
        /// Get the swapchain's format (backbuffer format).
        /// </summary>
        [[nodiscard]] virtual EResourceFormat GetFormat() = 0;

        /// <summary>
        /// Set swapchain's size.
        /// </summary>
        virtual void Resize(
            const Size2I&   Size,
            EResourceFormat NewFormat = EResourceFormat::Unknown) = 0;

        /// <summary>
        /// Get the swapchain's queue.
        /// </summary>
        [[nodiscard]] virtual ICommandQueue* GetQueue(
            CommandQueueType Type) = 0;

        /// <summary>
        /// Get the resource state manager.
        /// </summary>
        [[nodiscard]] virtual IResourceStateManager* GetStateManager() = 0;

        /// <summary>
        /// Get descriptor heap manager.
        /// </summary>
        [[nodiscard]] virtual IDescriptorHeapAllocator* GetDescriptorHeapManager(
            DescriptorType Type,
            bool           Dynamic) = 0;

        /// <summary>
        /// Enqueue descriptor heap to be released at the end of the frame.
        /// </summary>
        virtual void SafeRelease(
            const Ptr<IDescriptorHeap>& Heap) = 0;

        /// <summary>
        /// Enqueue resource to be released at the end of the frame.
        /// </summary>
        virtual void SafeRelease(
            const Ptr<IDescriptorHeapAllocator>& Allocator,
            const DescriptorHeapHandle&          Handle) = 0;
    };
} // namespace Neon::RHI