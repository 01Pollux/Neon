#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Resource/Common.hpp>
#include <Private/RHI/Dx12/Resource/State.hpp>
#include <Private/RHI/Dx12/Swapchain.hpp>

namespace Neon::RHI
{
    Dx12GpuResource::Dx12GpuResource(
        ISwapchain* Swapchain) :
        m_OwningSwapchain(Swapchain)
    {
    }

    ID3D12Resource* Dx12GpuResource::GetResource() const
    {
        return m_Resource.Get();
    }

    D3D12MA::Allocation* Dx12GpuResource::GetAllocation() const
    {
        return m_Allocation.Get();
    }

    //

    IBuffer* IBuffer::Create(
        ISwapchain*       Swapchain,
        const BufferDesc& Desc)
    {
        return NEON_NEW Dx12Buffer(Swapchain, Desc, GraphicsBufferType::Default);
    }

    Dx12Buffer::Dx12Buffer(
        ISwapchain*        Swapchain,
        const BufferDesc&  Desc,
        GraphicsBufferType Type) :
        Dx12GpuResource(Swapchain)
    {
        auto Allocator = static_cast<Dx12Swapchain*>(m_OwningSwapchain)->GetAllocator();
        m_Buffer       = Allocator->AllocateBuffer(Type, Desc.Size, size_t(Desc.Alignment), CastResourceFlags(Desc.Flags));
        m_Resource     = m_Buffer.Resource;
    }

    Dx12Buffer::~Dx12Buffer()
    {
        static_cast<Dx12Swapchain*>(m_OwningSwapchain)->SafeRelease(m_Buffer);
    }

    size_t Dx12Buffer::GetSize() const
    {
        return m_Buffer.Size;
    }

    GpuResourceHandle Dx12Buffer::GetHandle() const
    {
        return { m_Resource->GetGPUVirtualAddress() + m_Buffer.Offset };
    }
    //

    IUploadBuffer* IUploadBuffer::Create(
        ISwapchain*       Swapchain,
        const BufferDesc& Desc)
    {
        return NEON_NEW Dx12UploadBuffer(Swapchain, Desc, GraphicsBufferType::Upload);
    }

    Dx12UploadBuffer::Dx12UploadBuffer(
        ISwapchain*        Swapchain,
        const BufferDesc&  Desc,
        GraphicsBufferType Type) :
        Dx12Buffer(Swapchain, Desc, Type)
    {
    }

    uint8_t* Dx12UploadBuffer::Map()
    {
        void* MappedData = nullptr;
        ThrowIfFailed(m_Resource->Map(0, nullptr, &MappedData));
        return std::bit_cast<uint8_t*>(MappedData) + m_Buffer.Offset;
    }

    void Dx12UploadBuffer::Unmap()
    {
        m_Resource->Unmap(0, nullptr);
    }

    //

    IReadbackBuffer* IReadbackBuffer::Create(
        ISwapchain*       Swapchain,
        const BufferDesc& Desc)
    {
        return NEON_NEW Dx12ReadbackBuffer(Swapchain, Desc, GraphicsBufferType::Readback);
    }

    Dx12ReadbackBuffer::Dx12ReadbackBuffer(
        ISwapchain*        Swapchain,
        const BufferDesc&  Desc,
        GraphicsBufferType Type) :
        Dx12Buffer(Swapchain, Desc, Type)
    {
    }

    uint8_t* Dx12ReadbackBuffer::Map()
    {
        void* MappedData = nullptr;
        ThrowIfFailed(m_Resource->Map(0, nullptr, &MappedData));
        return std::bit_cast<uint8_t*>(MappedData) + m_Buffer.Offset;
    }

    void Dx12ReadbackBuffer::Unmap()
    {
        m_Resource->Unmap(0, nullptr);
    }

    //

    [[nodiscard]] ITexture* ITexture::Create(
        ISwapchain*         Swapchain,
        const ResourceDesc& Desc)
    {
        // TODO:
        return nullptr;
    }

    Dx12Texture::Dx12Texture(
        ISwapchain* Swapchain) :
        Dx12GpuResource(Swapchain)
    {
    }

    Dx12Texture::Dx12Texture(
        ISwapchain*                        Swapchain,
        Win32::ComPtr<ID3D12Resource>      Texture,
        D3D12_RESOURCE_STATES              InitialState,
        Win32::ComPtr<D3D12MA::Allocation> Allocation) :
        Dx12GpuResource(Swapchain)
    {
        m_Resource   = std::move(Texture);
        m_Allocation = std::move(Allocation);

        if (m_Resource)
        {
            auto Desc = m_Resource->GetDesc();

            m_Dimensions = { int(Desc.Width),
                             int(Desc.Height),
                             int(Desc.DepthOrArraySize) };
            m_MipLevels  = Desc.MipLevels;

            auto Dx12StateManager = static_cast<Dx12ResourceStateManager*>(m_OwningSwapchain->GetStateManager());
            Dx12StateManager->StartTrakingResource(m_Resource.Get(), InitialState);
        }
    }

    Dx12Texture::~Dx12Texture()
    {
        if (m_Resource)
        {
            auto Dx12StateManager = static_cast<Dx12ResourceStateManager*>(m_OwningSwapchain->GetStateManager());
            Dx12StateManager->StopTrakingResource(m_Resource.Get());
            static_cast<Dx12Swapchain*>(m_OwningSwapchain)->SafeRelease(m_Resource);
        }
    }

    const Vector3DI& Dx12Texture::GetDimensions() const
    {
        return m_Dimensions;
    }

    uint16_t Dx12Texture::GetMipLevels() const
    {
        return m_MipLevels;
    }
} // namespace Neon::RHI