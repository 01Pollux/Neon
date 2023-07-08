#pragma once

#include <Core/Neon.hpp>
#include <RHI/Resource/Descriptor.hpp>

namespace Neon::RHI
{
    /// <summary>
    /// A static descriptor heap is CPU only heap that is a descriptor heap that is persistent and can be used by multiple threads
    /// It is used for resources that are not frequently created and destroyed
    /// </summary>
    class IStaticDescriptorHeap
    {
    public:
        /// <summary>
        /// Get the descriptor heap
        /// </summary>
        [[nodiscard]] static IStaticDescriptorHeap* Get(
            DescriptorType Type);

        virtual ~IStaticDescriptorHeap() = default;

        /// <summary>
        /// Allocate a descriptor
        /// </summary>
        [[nodiscard]] virtual DescriptorHeapHandle Allocate(
            uint32_t Count) = 0;

        /// <summary>
        /// Free descriptor handles
        /// </summary>
        virtual void Free(
            std::span<const DescriptorHeapHandle> Handles) = 0;

        /// <summary>
        /// Free a descriptor handle
        /// </summary>
        void Free(
            const DescriptorHeapHandle& Handle)
        {
            Free({ &Handle, 1 });
        }
    };

    //

    /// <summary>
    /// A staged descriptor heap is a CPU only heap that is a descriptor heap that persists for a single frame and can be used by multiple threads
    /// It is used for resources that are frequently created and destroyed
    /// </summary>
    class IStagedDescriptorHeap
    {
    public:
        /// <summary>
        /// Get the descriptor heap
        /// </summary>
        [[nodiscard]] static IStagedDescriptorHeap* Get(
            DescriptorType Type);

        virtual ~IStagedDescriptorHeap() = default;

        /// <summary>
        /// Allocate a descriptor
        /// </summary>
        [[nodiscard]] virtual DescriptorHeapHandle Allocate(
            uint32_t Count) = 0;

        /// <summary>
        /// Free descriptor handles
        /// </summary>
        virtual void Free(
            std::span<const DescriptorHeapHandle> Handles) = 0;

        /// <summary>
        /// Free a descriptor handle
        /// </summary>
        void Free(
            const DescriptorHeapHandle& Handle)
        {
            Free({ &Handle, 1 });
        }
    };

    //

    /// <summary>
    /// A frame descriptor heap is a GPU-CPU heap that is a descriptor heap that persists for a single frame and can be used by multiple threads
    /// It is used for uploading resources to the GPU
    /// </summary>
    class IFrameDescriptorHeap
    {
    public:
        /// <summary>
        /// Get the descriptor heap
        /// </summary>
        [[nodiscard]] static IFrameDescriptorHeap* Get(
            DescriptorType Type);

        virtual ~IFrameDescriptorHeap() = default;

        /// <summary>
        /// Allocate a descriptor
        /// </summary>
        [[nodiscard]] virtual DescriptorHeapHandle Allocate(
            uint32_t Count) = 0;
    };
} // namespace Neon::RHI