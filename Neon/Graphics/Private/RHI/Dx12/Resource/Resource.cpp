#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Resource/Resource.hpp>
#include <Private/RHI/Dx12/Resource/State.hpp>
#include <Private/RHI/Dx12/Device.hpp>
#include <Private/RHI/Dx12/Swapchain.hpp>

namespace Neon::RHI
{

    Dx12GpuResource::Dx12GpuResource(
        ISwapchain* Swapchain) :
        IGpuResource(Swapchain)
    {
    }

    Dx12GpuResource::~Dx12GpuResource()
    {
        if (m_Resource)
        {
            auto Dx12StateManager = static_cast<Dx12ResourceStateManager*>(m_OwningSwapchain->GetStateManager());
            Dx12StateManager->StopTrakingResource(m_Resource.Get());
        }
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

    size_t Dx12Buffer::GetSize() const
    {
        return m_BufferSize;
    }

    //

    uint8_t* Dx12UploadBuffer::Map()
    {
        void* MappedData = nullptr;
        ThrowIfFailed(m_Buffer->Map(0, nullptr, &MappedData));
        return std::bit_cast<uint8_t*>(MappedData) + m_BufferOffset;
    }

    void Dx12UploadBuffer::Unmap()
    {
        m_Buffer->Unmap(0, nullptr);
    }

    //

    const uint8_t* Dx12ReadbackBuffer::Map()
    {
        void* MappedData = nullptr;
        ThrowIfFailed(m_Buffer->Map(0, nullptr, &MappedData));
        return std::bit_cast<const uint8_t*>(MappedData) + m_BufferOffset;
    }

    void Dx12ReadbackBuffer::Unmap()
    {
        m_Buffer->Unmap(0, nullptr);
    }

    //

    Dx12Texture::Dx12Texture(
        ISwapchain* Swapchain) :
        IGpuResource(Swapchain),
        Dx12GpuResource(Swapchain)
    {
    }

    Dx12Texture::Dx12Texture(
        ISwapchain*                        Swapchain,
        Win32::ComPtr<ID3D12Resource>      Texture,
        D3D12_RESOURCE_STATES              InitialState,
        Win32::ComPtr<D3D12MA::Allocation> Allocation) :
        IGpuResource(Swapchain),
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

    const Vector3DI& Dx12Texture::GetDimensions() const
    {
        return m_Dimensions;
    }

    uint16_t Dx12Texture::GetMipLevels() const
    {
        return m_MipLevels;
    }
} // namespace Neon::RHI