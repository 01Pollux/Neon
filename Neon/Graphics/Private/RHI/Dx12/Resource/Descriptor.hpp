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
            size_t                     MaxCount,
            bool                       ShaderVisible);

        /// <summary>
        /// Copy to descriptor
        /// </summary>
        void Copy(
            size_t          DescriptorIndex,
            const CopyInfo& SrcDescriptors) override;

        /// <summary>
        /// Get cpu handle of offset
        /// </summary>
        [[nodiscard]] CpuDescriptorHandle GetCPUAddress(
            size_t Offset = 0) override;

        /// <summary>
        /// Get heap descriptor
        /// </summary>
        [[nodiscard]] GpuDescriptorHandle GetGPUAddress(
            size_t Offset = 0) override;

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
            size_t         DescriptorIndex,
            const CBVDesc& Desc) override;

        /// <summary>
        /// Create shader resource view of target offset in current descriptor
        /// </summary>
        void CreateShaderResourceView(
            size_t         DescriptorIndex,
            IGpuResource*  Resource,
            const SRVDesc* Desc) override;

        /// <summary>
        /// Create unordered access view of target offset in current descriptor
        /// </summary>
        void CreateUnorderedAccessView(
            size_t         DescriptorIndex,
            IGpuResource*  Resource,
            const UAVDesc* Desc,
            IGpuResource*  CounterBuffer = nullptr) override;

        /// <summary>
        /// Create render target view of target offset in current descriptor
        /// </summary>
        void CreateRenderTargetView(
            size_t         DescriptorIndex,
            IGpuResource*  Resource,
            const RTVDesc* Desc) override;

        /// <summary>
        /// Create depth stencil view of target offset in current descriptor
        /// </summary>
        void CreateDepthStencilView(
            size_t         DescriptorIndex,
            IGpuResource*  Resource,
            const DSVDesc* Desc) override;

        /// <summary>
        /// Create sampler of target offset in current descriptor
        /// </summary>
        void CreateSampler(
            size_t             DescriptorIndex,
            const SamplerDesc& Desc) override;

    public:
        /// <summary>
        /// Get heap descriptor
        /// </summary>
        [[nodiscard]] ID3D12DescriptorHeap* Get() const noexcept;

        /// <summary>
        /// Get heap descriptor size
        /// </summary>
        [[nodiscard]] size_t GetSize() const noexcept;

        /// <summary>
        /// Get heap descriptor type
        /// </summary>
        [[nodiscard]] D3D12_DESCRIPTOR_HEAP_TYPE GetType() const noexcept;

    protected:
        Win32::ComPtr<ID3D12DescriptorHeap> m_DescriptorHeap;

        D3D12_CPU_DESCRIPTOR_HANDLE m_CpuHandle{};
        D3D12_GPU_DESCRIPTOR_HANDLE m_GpuHandle{};

        size_t                     m_HeapSize;
        D3D12_DESCRIPTOR_HEAP_TYPE m_HeapType;
    };

    //

    struct HeapDescriptorAllocInfo
    {
        size_t                     SizeOfHeap;
        D3D12_DESCRIPTOR_HEAP_TYPE DescriptorType;
        bool                       ShaderVisible;
    };

    class RingDescriptorHeapAllocator final : public IDescriptorHeapAllocator
    {
    public:
        RingDescriptorHeapAllocator(
            D3D12_DESCRIPTOR_HEAP_TYPE DescriptorType,
            size_t                     MaxCount,
            bool                       ShaderVisible);

        DescriptorHeapHandle Allocate(
            size_t DescriptorSize) override;

        void Free(
            const DescriptorHeapHandle& Data) override;

        void FreeAll() override;

        IDescriptorHeap* GetHeap(
            size_t) override;

    private:
        std::mutex m_DescriptorLock;

        Dx12DescriptorHeap m_HeapDescriptor;
        size_t             m_CurrentDescriptorOffset = 0;
    };

    //

    class DescriptorHeapBuddyAllocator final : public IDescriptorHeapAllocator
    {
        struct BuddyBlock
        {
            Dx12DescriptorHeap        Heap;
            Allocator::BuddyAllocator Allocator;

            BuddyBlock(
                const HeapDescriptorAllocInfo& Info);
        };

    public:
        DescriptorHeapBuddyAllocator(
            D3D12_DESCRIPTOR_HEAP_TYPE DescriptorType,
            size_t                     SizeOfHeap,
            bool                       ShaderVisible);

        DescriptorHeapHandle Allocate(
            size_t DescriptorSize) override;

        void Free(
            const DescriptorHeapHandle& Data) override;

        void FreeAll() override;

        IDescriptorHeap* GetHeap(
            size_t Index) override;

    private:
        std::mutex              m_HeapsBlockMutex;
        HeapDescriptorAllocInfo m_HeapBlockAllocInfo;
        std::list<BuddyBlock>   m_HeapBlocks;
    };
} // namespace Neon::RHI