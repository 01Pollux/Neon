#pragma once

#include <RHI/Resource/View.hpp>
#include <span>

namespace Neon::RHI
{
    class ISwapchain;

    enum class DescriptorType : uint8_t
    {
        ResourceView,
        RenderTargetView,
        DepthStencilView,
        Sampler,
    };

    class IDescriptorHeap
    {
    public:
        struct CopyInfo
        {
            CpuDescriptorHandle Descriptor;
            uint32_t            CopySize;
        };

    public:
        [[nodiscard]] static IDescriptorHeap* Create(
            ISwapchain*    Swapchain,
            DescriptorType Type,
            uint32_t       MaxCount,
            bool           ShaderVisible);

        virtual ~IDescriptorHeap() = default;

        /// <summary>
        /// Copy to descriptor
        /// </summary>
        virtual void Copy(
            uint32_t        DescriptorIndex,
            const CopyInfo& SrcDescriptors) = 0;

        /// <summary>
        /// Copy to descriptor
        /// </summary>
        static void Copy(
            DescriptorType            DescriptorType,
            std::span<const CopyInfo> SrcDescriptors,
            std::span<const CopyInfo> DstDescriptors);

        /// <summary>
        /// Get cpu handle of offset
        /// </summary>
        [[nodiscard]] virtual CpuDescriptorHandle GetCPUAddress(
            uint32_t Offset = 0) = 0;

        /// <summary>
        /// Get heap descriptor
        /// </summary>
        [[nodiscard]] virtual GpuDescriptorHandle GetGPUAddress(
            uint32_t Offset = 0) = 0;

        /// <summary>
        /// Check to see if the descriptor is in range
        /// </summary>
        [[nodiscard]] virtual bool IsDescriptorInRange(
            CpuDescriptorHandle Handle) = 0;

        /// <summary>
        /// Check to see if the descriptor is in range
        /// </summary>
        [[nodiscard]] virtual bool IsDescriptorInRange(
            GpuDescriptorHandle Handle) = 0;

    public:
        /// <summary>
        /// Create constant buffer view of target offset in current descriptor
        /// </summary>
        virtual void CreateConstantBufferView(
            uint32_t       DescriptorIndex,
            const CBVDesc& Desc) = 0;

        /// <summary>
        /// Create shader resource view of target offset in current descriptor
        /// </summary>
        virtual void CreateShaderResourceView(
            uint32_t       DescriptorIndex,
            IGpuResource*  Resource = nullptr,
            const SRVDesc* Desc     = nullptr) = 0;

        /// <summary>
        /// Create unordered access view of target offset in current descriptor
        /// </summary>
        virtual void CreateUnorderedAccessView(
            uint32_t       DescriptorIndex,
            IGpuResource*  Resource      = nullptr,
            const UAVDesc* Desc          = nullptr,
            IGpuResource*  CounterBuffer = nullptr) = 0;

        /// <summary>
        /// Create render target view of target offset in current descriptor
        /// </summary>
        virtual void CreateRenderTargetView(
            uint32_t       DescriptorIndex,
            IGpuResource*  Resource = nullptr,
            const RTVDesc* Desc     = nullptr) = 0;

        /// <summary>
        /// Create depth stencil view of target offset in current descriptor
        /// </summary>
        virtual void CreateDepthStencilView(
            uint32_t       DescriptorIndex,
            IGpuResource*  Resource = nullptr,
            const DSVDesc* Desc     = nullptr) = 0;

        /// <summary>
        /// Create sampler of target offset in current descriptor
        /// </summary>
        virtual void CreateSampler(
            uint32_t           DescriptorIndex,
            const SamplerDesc& Desc) = 0;
    };

    //

    struct DescriptorHeapHandle
    {
        IDescriptorHeap* Heap   = nullptr;
        uint32_t         Offset = std::numeric_limits<uint32_t>::max();
        uint32_t         Size   = 0;

        constexpr operator bool() const noexcept
        {
            return Heap != nullptr;
        }

        [[nodiscard]] IDescriptorHeap* operator->() const noexcept
        {
            return Heap;
        }
    };

    //

    class IDescriptorHeapAllocator
    {
    public:
        virtual ~IDescriptorHeapAllocator() = default;

        enum class AllocationType : uint8_t
        {
            Ring,
            Buddy
        };

        [[nodiscard]] static IDescriptorHeapAllocator* Create(
            ISwapchain*    Swapchain,
            AllocationType Type,
            DescriptorType DescType,
            uint32_t       SizeOfHeap,
            bool           ShaderVisible);

        /// <summary>
        /// Allocate descriptor handle of size
        /// </summary>
        [[nodiscard]] virtual DescriptorHeapHandle Allocate(
            uint32_t DescriptorSize) = 0;

        /// <summary>
        /// Free current descriptor handles
        /// </summary>
        virtual void Free(
            std::span<DescriptorHeapHandle> Handles) = 0;

        /// <summary>
        /// Free current descriptor handle
        /// </summary>
        void Free(
            const DescriptorHeapHandle& Data)
        {
            std::array Handles(Data);
            Free(Handles);
        }

        /// <summary>
        /// Free all active descriptors
        /// </summary>
        virtual void FreeAll() = 0;

        /// <summary>
        /// Get heap at index
        /// for ring allocator, index is always 0
        /// for buddy allocator, index is the level of heap
        /// </summary>
        [[nodiscard]] virtual IDescriptorHeap* GetHeap(
            uint32_t Index = 0) = 0;
    };
} // namespace Neon::RHI