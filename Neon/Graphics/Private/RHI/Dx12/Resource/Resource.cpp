#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Resource/Resource.hpp>

namespace Neon::RHI
{
    size_t Dx12Buffer::GetSize() const
    {
        return m_BufferSize;
    }

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

    const uint8_t* Dx12ReadbackBuffer::Map()
    {
        void* MappedData = nullptr;
        ThrowIfFailed(m_Buffer->Map(0, nullptr, &MappedData));
        return std::bit_cast<const uint8_t*>(MappedData) + m_BufferOffset;
    }

    void Dx12ReadbackBuffer::Unmap()
    {
    }
    const Vector3D& Dx12Texture::GetDimensions() const
    {
        // TODO: insert return statement here
    }
    uint32_t Dx12Texture::GetMipLevels() const
    {
        return 0;
    }
} // namespace Neon::RHI