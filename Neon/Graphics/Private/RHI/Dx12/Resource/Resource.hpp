#pragma once

#include <RHI/Resource/Resource.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>

#include <Math/Vector.hpp>

namespace Neon::RHI
{

    enum class GraphicsBufferType : uint8_t
    {
        Default,
        Upload,
        Readback,

        Count
    };

    class Dx12GpuResource : public virtual IGpuResource
    {
    public:
        Dx12GpuResource(
            ISwapchain* Swapchain);

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

        ~Dx12Buffer() override;

        [[nodiscard]] size_t GetSize() const override;

        GpuResourceHandle GetHandle() const override;

    protected:
        Handle m_Buffer;
    };

    //

    class Dx12UploadBuffer : public virtual IUploadBuffer,
                             public Dx12Buffer
    {
    public:
        Dx12UploadBuffer(
            ISwapchain*        Swapchain,
            const BufferDesc&  Desc,
            GraphicsBufferType Type);

        [[nodiscard]] uint8_t* Map() override;

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
            ISwapchain*        Swapchain,
            const BufferDesc&  Desc,
            GraphicsBufferType Type);

        [[nodiscard]] uint8_t* Map() override;

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
            ISwapchain* Swapchain);

        Dx12Texture(
            ISwapchain*                        Swapchain,
            Win32::ComPtr<ID3D12Resource>      Texture,
            D3D12_RESOURCE_STATES              InitialState,
            Win32::ComPtr<D3D12MA::Allocation> Allocation = nullptr);

        ~Dx12Texture() override;

        [[nodiscard]] const Vector3DI& GetDimensions() const override;

        [[nodiscard]] uint16_t GetMipLevels() const override;

    protected:
        Vector3DI m_Dimensions;
        uint16_t  m_MipLevels = 0;
    };
} // namespace Neon::RHI