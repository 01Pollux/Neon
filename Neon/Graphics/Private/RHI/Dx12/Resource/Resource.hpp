#pragma once

#include <RHI/Resource/Resource.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>
#include <Math/Vector.hpp>
#include <future>

namespace Neon::RHI
{
    class Dx12GpuResource : public virtual IGpuResource
    {
    public:
        ~Dx12GpuResource();

        void QueryFootprint(
            uint32_t              FirstSubresource,
            uint32_t              SubresourceCount,
            size_t                Offset,
            SubresourceFootprint* OutFootprint,
            uint32_t*             NumRows,
            size_t*               RowSizeInBytes,
            size_t*               LinearSize) const override;

        void CopyFrom(
            uint32_t                           FirstSubresource,
            std::span<const SubresourceDesc>   Subresources,
            uint64_t&                          CopyId,
            std::optional<RHI::MResourceState> TransitionState) override;

        /// <summary>
        /// Get the underlying D3D12 resource.
        /// </summary>
        [[nodiscard]] ID3D12Resource* GetResource() const;

        /// <summary>
        /// Get the underlying D3D12 resource allocation.
        /// </summary>
        [[nodiscard]] D3D12MA::Allocation* GetAllocation() const;

        /// <summary>
        /// Release the underlying D3D12 resource without enqueueing a delete.
        /// </summary>
        void SilentRelease();

    protected:
        WinAPI::ComPtr<ID3D12Resource>      m_Resource;
        WinAPI::ComPtr<D3D12MA::Allocation> m_Allocation;
    };

    class Dx12Buffer : public virtual IBuffer,
                       public Dx12GpuResource
    {
        friend class FrameResource;

    public:
        Dx12Buffer(
            const BufferDesc&          Desc,
            const SubresourceDesc*     Subresource,
            uint64_t*                  CopyId,
            GraphicsBufferType         Type,
            const RHI::MResourceState& InitialState);

        Dx12Buffer(
            size_t                     Size,
            D3D12_RESOURCE_FLAGS       Flags,
            GraphicsBufferType         Type,
            const RHI::MResourceState& InitialState);

        size_t GetSize() const override;

        GpuResourceHandle GetHandle(
            size_t Offset) const override;

    protected:
        GraphicsBufferType m_Type{};
    };

    //

    class Dx12UploadBuffer : public virtual IUploadBuffer,
                             public Dx12Buffer
    {
    public:
        Dx12UploadBuffer(
            const BufferDesc&          Desc,
            const SubresourceDesc*     Subresource,
            uint64_t*                  CopyId,
            const RHI::MResourceState& InitialState);

        Dx12UploadBuffer(
            size_t                     Size,
            D3D12_RESOURCE_FLAGS       Flags,
            const RHI::MResourceState& InitialState);

        uint8_t* Map() override;

        void Unmap() override;
    };

    //

    class Dx12ReadbackBuffer : public virtual IReadbackBuffer,
                               public Dx12Buffer
    {
    public:
        Dx12ReadbackBuffer(
            const BufferDesc&          Desc,
            const RHI::MResourceState& InitialState);

        Dx12ReadbackBuffer(
            size_t                     Size,
            D3D12_RESOURCE_FLAGS       Flags,
            const RHI::MResourceState& InitialState);

        uint8_t* Map() override;

        void Unmap() override;
    };

    //

    class Dx12Texture : public virtual ITexture,
                        public Dx12GpuResource
    {
    public:
        Dx12Texture(
            const RHI::ResourceDesc&         Desc,
            std::span<const SubresourceDesc> Subresources,
            uint64_t*                        CopyId,
            const RHI::MResourceState&       InitialState);

        Dx12Texture(
            WinAPI::ComPtr<ID3D12Resource>      Texture,
            D3D12_RESOURCE_STATES               InitialState,
            WinAPI::ComPtr<D3D12MA::Allocation> Allocation = nullptr);

        Dx12Texture(
            WinAPI::ComPtr<ID3D12Resource>      Texture,
            WinAPI::ComPtr<D3D12MA::Allocation> Allocation,
            std::span<const SubresourceDesc>    Subresources,
            uint64_t&                           CopyId,
            const RHI::MResourceState&          InitialState);

        Vector3I GetDimensions() const override;

        uint16_t GetMipLevels() const override;

        uint32_t GetSubResourceCount() const override;

        uint32_t GetSubresourceIndex(
            uint32_t PlaneIndex,
            uint32_t ArrayIndex,
            uint32_t MipIndex) const override;

    private:
        /// <summary>
        /// Initialize the texture description.
        /// </summary>
        void InitializeDesc();

        /// <summary>
        /// Get the size of the texture in bytes to copy.
        /// </summary>
        [[nodiscard]] size_t GetTextureCopySize(
            uint32_t SubresourcesCount);

    protected:
        ClearOperationOpt m_ClearValue;
    };
} // namespace Neon::RHI