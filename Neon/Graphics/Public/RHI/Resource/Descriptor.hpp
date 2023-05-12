#pragma once

#include <RHI/Resource/View.hpp>
#include <span>

namespace Neon::RHI
{
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
            size_t              CopySize;
        };

    public:
        [[nodiscard]] static IDescriptorHeap* Create(
            DescriptorType Type,
            size_t         MaxCount,
            bool           ShaderVisible);

        virtual ~IDescriptorHeap() = default;

        /// <summary>
        /// Copy to descriptor
        /// </summary>
        virtual void Copy(
            size_t          DescriptorIndex,
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
            size_t Offset = 0) = 0;

        /// <summary>
        /// Get heap descriptor
        /// </summary>
        [[nodiscard]] virtual GpuDescriptorHandle GetGPUAddress(
            size_t Offset = 0) = 0;

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
            size_t         DescriptorIndex,
            const CBVDesc& Desc) = 0;

        /// <summary>
        /// Create shader resource view of target offset in current descriptor
        /// </summary>
        virtual void CreateShaderResourceView(
            size_t         DescriptorIndex,
            IGpuResource*  Resource = nullptr,
            const SRVDesc* Desc     = nullptr) = 0;

        /// <summary>
        /// Create unordered access view of target offset in current descriptor
        /// </summary>
        virtual void CreateUnorderedAccessView(
            size_t         DescriptorIndex,
            IGpuResource*  Resource      = nullptr,
            const UAVDesc* Desc          = nullptr,
            IGpuResource*  CounterBuffer = nullptr) = 0;

        /// <summary>
        /// Create render target view of target offset in current descriptor
        /// </summary>
        virtual void CreateRenderTargetView(
            size_t         DescriptorIndex,
            IGpuResource*  Resource = nullptr,
            const RTVDesc* Desc     = nullptr) = 0;

        /// <summary>
        /// Create depth stencil view of target offset in current descriptor
        /// </summary>
        virtual void CreateDepthStencilView(
            size_t         DescriptorIndex,
            IGpuResource*  Resource = nullptr,
            const DSVDesc* Desc     = nullptr) = 0;

        /// <summary>
        /// Create sampler of target offset in current descriptor
        /// </summary>
        virtual void CreateSampler(
            size_t             DescriptorIndex,
            const SamplerDesc& Desc) = 0;
    };

    //

    struct DescriptorHeapHandle
    {
        IDescriptorHeap* Heap   = nullptr;
        size_t           Offset = std::numeric_limits<size_t>::max();
        size_t           Size   = 0;

        operator bool() const noexcept
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
            Buddy,
            // give me more strategies
        };

        [[nodiscard]] static IDescriptorHeapAllocator* Create(
            AllocationType Type,
            DescriptorType DescType,
            size_t         SizeOfHeap,
            bool           ShaderVisible);

        /// <summary>
        /// Allocate descriptor handle of size
        /// </summary>
        [[nodiscard]] virtual DescriptorHeapHandle Allocate(
            size_t DescriptorSize) = 0;

        /// <summary>
        /// Free current descriptor handle
        /// </summary>
        virtual void Free(
            const DescriptorHeapHandle& Data) = 0;

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
            size_t Index = 0) = 0;
    };
} // namespace Neon::RHI