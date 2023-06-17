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
        Dx12GpuResource(
            ISwapchain* Swapchain);

        void QueryFootprint(
            uint32_t              FirstSubresource,
            uint32_t              SubresourceCount,
            size_t                Offset,
            SubresourceFootprint* OutFootprint,
            uint32_t*             NumRows,
            size_t*               RowSizeInBytes,
            size_t*               TotalBytes) const;

        /// <summary>
        /// Get the underlying D3D12 resource.
        /// </summary>
        [[nodiscard]] ID3D12Resource* GetResource() const;

        /// <summary>
        /// Get the underlying D3D12 resource allocation.
        /// </summary>
        [[nodiscard]] D3D12MA::Allocation* GetAllocation() const;

    protected:
        ISwapchain*                        m_OwningSwapchain = nullptr;
        Win32::ComPtr<ID3D12Resource>      m_Resource;
        Win32::ComPtr<D3D12MA::Allocation> m_Allocation;
    };

    class Dx12Buffer : public virtual IBuffer,
                       public Dx12GpuResource
    {
        friend class FrameResource;

    public:
        struct Handle
        {
            Win32::ComPtr<ID3D12Resource> Resource;
            size_t                        Offset;
            size_t                        Size;
            GraphicsBufferType            Type;
            D3D12_RESOURCE_FLAGS          Flags;
        };

        Dx12Buffer(
            ISwapchain*        Swapchain,
            const BufferDesc&  Desc,
            GraphicsBufferType Type);

        NEON_CLASS_NO_COPY(Dx12Buffer);
        NEON_CLASS_MOVE(Dx12Buffer);

        ~Dx12Buffer() override;

        ResourceDesc GetDesc() const override;

        size_t GetSize() const override;

        GpuResourceHandle GetHandle() const override;

    protected:
        Handle   m_Buffer;
        uint32_t m_Alignement;
    };

    //

    class Dx12UploadBuffer : public virtual IUploadBuffer,
                             public Dx12Buffer
    {
    public:
        Dx12UploadBuffer(
            ISwapchain*       Swapchain,
            const BufferDesc& Desc);

        uint8_t* Map() override;

        void Unmap() override;

    private:
        uint8_t* m_MappedData = nullptr;
    };

    //

    class Dx12ReadbackBuffer : public virtual IReadbackBuffer,
                               public Dx12Buffer
    {
    public:
        Dx12ReadbackBuffer(
            ISwapchain*       Swapchain,
            const BufferDesc& Desc);

        uint8_t* Map() override;

        void Unmap() override;

    private:
        uint8_t* m_MappedData = nullptr;
    };

    //

    class Dx12Texture : public virtual ITexture,
                        public Dx12GpuResource
    {
    public:
        Dx12Texture(
            ISwapchain*                      Swapchain,
            const RHI::ResourceDesc&         Desc,
            std::span<const SubresourceDesc> Subresources,
            uint64_t*                        CopyId);

        Dx12Texture(
            ISwapchain*                        Swapchain,
            Win32::ComPtr<ID3D12Resource>      Texture,
            D3D12_RESOURCE_STATES              InitialState,
            Win32::ComPtr<D3D12MA::Allocation> Allocation = nullptr);

        Dx12Texture(
            ISwapchain*                        Swapchain,
            Win32::ComPtr<ID3D12Resource>      Texture,
            Win32::ComPtr<D3D12MA::Allocation> Allocation,
            std::span<const SubresourceDesc>   Subresources,
            uint64_t&                          CopyId);

        NEON_CLASS_NO_COPY(Dx12Texture);
        NEON_CLASS_MOVE(Dx12Texture);

        ~Dx12Texture() override;

        ResourceDesc GetDesc() const override;

        const Vector3DI& GetDimensions() const override;

        uint16_t GetMipLevels() const override;

        uint32_t GetSubResourceCount() const override;

        uint32_t GetSubresourceIndex(
            uint32_t PlaneIndex,
            uint32_t ArrayIndex,
            uint32_t MipIndex) const override;

    private:
        /// <summary>
        /// Get the size of the texture in bytes to copy.
        /// </summary>
        [[nodiscard]] size_t GetTextureCopySize(
            uint32_t SubresourcesCount);

        /// <summary>
        /// Copy the subresources to the texture.
        /// </summary>
        void CopyFrom(
            std::span<const SubresourceDesc> Subresources,
            uint64_t&                        CopyId);

    protected:
        Vector3DI m_Dimensions;
        uint16_t  m_MipLevels = 0;

        ClearOperationOpt m_ClearValue;
    };
} // namespace Neon::RHI