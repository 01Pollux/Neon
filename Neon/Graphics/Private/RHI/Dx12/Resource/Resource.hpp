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
        void QueryFootprint(
            uint32_t              FirstSubresource,
            uint32_t              SubresourceCount,
            size_t                Offset,
            SubresourceFootprint* OutFootprint,
            uint32_t*             NumRows,
            size_t*               RowSizeInBytes,
            size_t*               TotalBytes) const;

        const ResourceDesc& GetDesc() const override;

        /// <summary>
        /// Get the underlying D3D12 resource.
        /// </summary>
        [[nodiscard]] ID3D12Resource* GetResource() const;

        /// <summary>
        /// Get the underlying D3D12 resource allocation.
        /// </summary>
        [[nodiscard]] D3D12MA::Allocation* GetAllocation() const;

    protected:
        WinAPI::ComPtr<ID3D12Resource>      m_Resource;
        WinAPI::ComPtr<D3D12MA::Allocation> m_Allocation;
        ResourceDesc                        m_Desc;
    };

    class Dx12Buffer : public virtual IBuffer,
                       public Dx12GpuResource
    {
        friend class FrameResource;

    public:
        struct Handle
        {
            WinAPI::ComPtr<ID3D12Resource> Resource;
            size_t                         Offset;
            size_t                         Size;
            GraphicsBufferType             Type;
            D3D12_RESOURCE_FLAGS           Flags;
        };

        Dx12Buffer(
            const BufferDesc&  Desc,
            GraphicsBufferType Type);

        NEON_CLASS_NO_COPY(Dx12Buffer);
        NEON_CLASS_MOVE(Dx12Buffer);

        ~Dx12Buffer() override;

        size_t GetSize() const override;

        GpuResourceHandle GetHandle(
            size_t Offset) const override;

    protected:
        /// <summary>
        /// Get the offset of the buffer in the allocation.
        /// </summary>
        [[nodiscard]] size_t GetOffset() const;

        /// <summary>
        /// Check if the buffer was created using a pool.
        /// </summary>
        bool IsUsingPool() const;

    protected:
        size_t m_Offset = 0;

        GraphicsBufferType m_Type{};
    };

    //

    class Dx12UploadBuffer : public virtual IUploadBuffer,
                             public Dx12Buffer
    {
    public:
        Dx12UploadBuffer(
            const BufferDesc& Desc);

        uint8_t* Map() override;

        void Unmap() override;
    };

    //

    class Dx12ReadbackBuffer : public virtual IReadbackBuffer,
                               public Dx12Buffer
    {
    public:
        Dx12ReadbackBuffer(
            const BufferDesc& Desc);

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
            uint64_t*                        CopyId);

        Dx12Texture(
            WinAPI::ComPtr<ID3D12Resource>      Texture,
            D3D12_RESOURCE_STATES               InitialState,
            WinAPI::ComPtr<D3D12MA::Allocation> Allocation = nullptr);

        Dx12Texture(
            WinAPI::ComPtr<ID3D12Resource>      Texture,
            WinAPI::ComPtr<D3D12MA::Allocation> Allocation,
            std::span<const SubresourceDesc>    Subresources,
            uint64_t&                           CopyId);

        NEON_CLASS_NO_COPY(Dx12Texture);
        NEON_CLASS_MOVE(Dx12Texture);

        ~Dx12Texture() override;

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

        /// <summary>
        /// Copy the subresources to the texture.
        /// </summary>
        void CopyFrom(
            std::span<const SubresourceDesc> Subresources,
            uint64_t&                        CopyId);

    protected:
        ClearOperationOpt m_ClearValue;
    };
} // namespace Neon::RHI