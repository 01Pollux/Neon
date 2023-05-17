#pragma once

#include <RHI/Resource/Resource.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>

#include <Math/Vector.hpp>

namespace Neon::RHI
{
    class Dx12GpuResource : public virtual IGpuResource
    {
    public:
        Dx12GpuResource(
            ISwapchain* Swapchain);
        ~Dx12GpuResource() override;

        /// <summary>
        /// Get the underlying D3D12 resource.
        /// </summary>
        [[nodiscard]] ID3D12Resource* GetResource() const;

        /// <summary>
        /// Get the underlying D3D12 resource allocation.
        /// </summary>
        [[nodiscard]] D3D12MA::Allocation* GetAllocation() const;

    protected:
        Win32::ComPtr<ID3D12Resource>      m_Resource;
        Win32::ComPtr<D3D12MA::Allocation> m_Allocation;
    };

    class Dx12Buffer : public virtual IBuffer,
                       public Dx12GpuResource
    {
    public:
        Dx12Buffer(
            ISwapchain* Swapchain,
            const Desc& Desc);

        [[nodiscard]] size_t GetSize() const override;

    protected:
        Win32::ComPtr<ID3D12Resource> m_Buffer;

        size_t m_BufferSize;
        size_t m_BufferOffset;
    };

    //

    class Dx12UploadBuffer : public virtual IUploadBuffer,
                             public Dx12Buffer
    {
    public:
        [[nodiscard]] uint8_t* Map() override;

        void Unmap() override;

    private:
        uint8_t* m_MappedData;
    };

    //

    class Dx12ReadbackBuffer : public virtual IReadbackBuffer,
                               public Dx12Buffer
    {
    public:
        [[nodiscard]] const uint8_t* Map() override;

        void Unmap() override;

    private:
        uint8_t* m_MappedData;
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

        [[nodiscard]] const Vector3DI& GetDimensions() const override;

        [[nodiscard]] uint16_t GetMipLevels() const override;

    protected:
        Vector3DI m_Dimensions;
        uint16_t  m_MipLevels = 0;
    };
} // namespace Neon::RHI