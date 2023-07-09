#pragma once

#include <GraphicsPCH.hpp>
#include <RHI/Resource/Descriptor.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>
#include <Allocator/Buddy.hpp>

namespace Neon::RHI
{
    /// <summary>
    /// Convert Neon descriptor type to D3D12 descriptor type.
    /// </summary>
    DescriptorType CastDescriptorType(
        D3D12_DESCRIPTOR_HEAP_TYPE Type);

    /// <summary>
    /// Convert D3D12 descriptor type to Neon descriptor type.
    /// </summary>
    D3D12_DESCRIPTOR_HEAP_TYPE CastDescriptorType(
        DescriptorType Type);

    //

    class Dx12DescriptorHeap final : public IDescriptorHeap
    {
    public:
        Dx12DescriptorHeap(
            D3D12_DESCRIPTOR_HEAP_TYPE Type,
            uint32_t                   MaxCount,
            bool                       ShaderVisible);

        NEON_CLASS_NO_COPY(Dx12DescriptorHeap);
        NEON_CLASS_MOVE(Dx12DescriptorHeap);

        ~Dx12DescriptorHeap() override;

        /// <summary>
        /// Copy to descriptor
        /// </summary>
        void Copy(
            uint32_t        DescriptorIndex,
            const CopyInfo& SrcDescriptors) override;

        /// <summary>
        /// Get cpu handle of offset
        /// </summary>
        [[nodiscard]] CpuDescriptorHandle GetCPUAddress(
            uint32_t Offset = 0) override;

        /// <summary>
        /// Get heap descriptor
        /// </summary>
        [[nodiscard]] GpuDescriptorHandle GetGPUAddress(
            uint32_t Offset = 0) override;

        /// <summary>
        /// Check to see if the descriptor is in range
        /// </summary>
        [[nodiscard]] bool IsDescriptorInRange(
            CpuDescriptorHandle Handle) override;

        /// <summary>
        /// Check to see if the descriptor is in range
        /// </summary>
        [[nodiscard]] bool IsDescriptorInRange(
            GpuDescriptorHandle Handle) override;

    public:
        /// <summary>
        /// Create constant buffer view of target offset in current descriptor
        /// </summary>
        void CreateConstantBufferView(
            uint32_t       DescriptorIndex,
            const CBVDesc& Desc) override;

        /// <summary>
        /// Create shader resource view of target offset in current descriptor
        /// </summary>
        void CreateShaderResourceView(
            uint32_t       DescriptorIndex,
            IGpuResource*  Resource,
            const SRVDesc* Desc) override;

        /// <summary>
        /// Create unordered access view of target offset in current descriptor
        /// </summary>
        void CreateUnorderedAccessView(
            uint32_t       DescriptorIndex,
            IGpuResource*  Resource,
            const UAVDesc* Desc,
            IGpuResource*  CounterBuffer = nullptr) override;

        /// <summary>
        /// Create render target view of target offset in current descriptor
        /// </summary>
        void CreateRenderTargetView(
            uint32_t       DescriptorIndex,
            IGpuResource*  Resource,
            const RTVDesc* Desc) override;

        /// <summary>
        /// Create depth stencil view of target offset in current descriptor
        /// </summary>
        void CreateDepthStencilView(
            uint32_t       DescriptorIndex,
            IGpuResource*  Resource,
            const DSVDesc* Desc) override;

        /// <summary>
        /// Create sampler of target offset in current descriptor
        /// </summary>
        void CreateSampler(
            uint32_t           DescriptorIndex,
            const SamplerDesc& Desc) override;

    public:
        /// <summary>
        /// Get heap descriptor
        /// </summary>
        [[nodiscard]] ID3D12DescriptorHeap* Get() const noexcept;

        /// <summary>
        /// Get heap descriptor size
        /// </summary>
        [[nodiscard]] uint32_t GetSize() const noexcept;

        /// <summary>
        /// Get heap descriptor type
        /// </summary>
        [[nodiscard]] D3D12_DESCRIPTOR_HEAP_TYPE GetType() const noexcept;

        /// <summary>
        /// Free descriptor without notifying swapchain to release
        /// </summary>
        void SilentDelete() noexcept;

    protected:
        Win32::ComPtr<ID3D12DescriptorHeap> m_DescriptorHeap;

        D3D12_CPU_DESCRIPTOR_HANDLE m_CpuHandle{};
        D3D12_GPU_DESCRIPTOR_HANDLE m_GpuHandle{};

        uint32_t                   m_HeapSize;
        D3D12_DESCRIPTOR_HEAP_TYPE m_HeapType;
    };

    //

    struct HeapDescriptorAllocInfo
    {
        uint32_t                   SizeOfHeap;
        D3D12_DESCRIPTOR_HEAP_TYPE DescriptorType;
        bool                       ShaderVisible;
    };

    class Dx12RingDescriptorHeapAllocator final : public IDescriptorHeapAllocator
    {
    public:
        Dx12RingDescriptorHeapAllocator(
            D3D12_DESCRIPTOR_HEAP_TYPE DescriptorType,
            uint32_t                   MaxCount,
            bool                       ShaderVisible);

        DescriptorHeapHandle Allocate(
            uint32_t DescriptorSize) override;

        void Free(
            std::span<const DescriptorHeapHandle>) override
        {
        }

        void FreeAll() override
        {
        }

        IDescriptorHeap* GetHeap(
            uint32_t) override;

        uint32_t GetHeapsCount() override;

    private:
        std::mutex         m_DescriptorLock;
        Dx12DescriptorHeap m_HeapDescriptor;
        uint32_t           m_CurrentDescriptorOffset = 0;
    };

    //

    class Dx12DescriptorHeapBuddyAllocator final : public IDescriptorHeapAllocator
    {
        struct BuddyBlock
        {
            Dx12DescriptorHeap        Heap;
            Allocator::BuddyAllocator Allocator;

            BuddyBlock(
                const HeapDescriptorAllocInfo& Info);
        };

    public:
        Dx12DescriptorHeapBuddyAllocator(
            D3D12_DESCRIPTOR_HEAP_TYPE DescriptorType,
            uint32_t                   SizeOfHeap,
            bool                       ShaderVisible);

        DescriptorHeapHandle Allocate(
            uint32_t DescriptorSize) override;

        void Free(
            std::span<const DescriptorHeapHandle> Handles) override;

        void FreeAll() override;

        IDescriptorHeap* GetHeap(
            uint32_t Index) override;

        uint32_t GetHeapsCount() override;

    public:
        /// <summary>
        /// Helper function to get all heaps in this allocator
        /// </summary>
        auto GetAllHeaps() noexcept
        {
            return m_HeapBlocks |
                   std::views::transform(
                       [](auto& Block) -> Dx12DescriptorHeap*
                       {
                           return &Block.Heap;
                       });
        }

    private:
        std::mutex              m_HeapBlocksMutex;
        HeapDescriptorAllocInfo m_HeapBlockAllocInfo;
        std::list<BuddyBlock>   m_HeapBlocks;
    };
} // namespace Neon::RHI