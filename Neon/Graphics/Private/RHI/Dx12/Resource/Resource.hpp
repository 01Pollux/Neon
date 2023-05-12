#pragma once

#include <RHI/Resource/Resource.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>

#include <Math/Vector.hpp>

namespace Neon::RHI
{
    class Dx12Buffer : public virtual IBuffer
    {
    public:
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

    class Dx12Texture : public ITexture
    {
    public:
        [[nodiscard]] const Vector3D& GetDimensions() const override;

        [[nodiscard]] uint8_t GetMipLevels() const override;

    protected:
        Win32::ComPtr<ID3D12Resource> m_Texture;

        Vector3D m_Dimensions;
        uint8_t  m_MipLevels;
    };
} // namespace Neon::RHI