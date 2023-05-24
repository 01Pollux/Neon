#pragma once

#include <RHI/Resource/Common.hpp>
#include <variant>

namespace Neon
{
    class Vector3DI;
}

namespace Neon::RHI
{
    class ISwapchain;

    //

    struct ResourceFootprint
    {
        uint32_t Width;
        uint32_t Height;
        uint32_t Depth;
        uint32_t RowPitch;

        EResourceFormat Format;
    };

    struct SubresourceFootprint : public ResourceFootprint
    {
        size_t Offset;
    };

    struct SubresourceDesc
    {
        const void* Data;
        size_t      RowPitch;
        size_t      SlicePitch;
    };

    struct TextureCopyLocation
    {
        IGpuResource*                                Resource;
        std::variant<uint32_t, SubresourceFootprint> Subresource;
    };
    ;

    //

    class IGpuResource
    {
    public:
        virtual ~IGpuResource() = default;
    };

    //

    struct BufferDesc
    {
        size_t         Size;
        uint32_t       Alignment = 1;
        MResourceFlags Flags;
    };

    class IBuffer : public virtual IGpuResource
    {
    public:
        using IGpuResource::IGpuResource;

        /// <summary>
        /// Creates a buffer.
        /// </summary>
        [[nodiscard]] static IBuffer* Create(
            ISwapchain*       Swapchain,
            const BufferDesc& Desc);

        /// <summary>
        /// Get the size of the buffer in bytes.
        /// </summary>
        [[nodiscard]] virtual size_t GetSize() const = 0;

        /// <summary>
        /// Get resource handle in gpu.
        /// </summary>
        [[nodiscard]] virtual GpuResourceHandle GetHandle() const = 0;
    };

    //

    class IUploadBuffer : public virtual IBuffer
    {
    public:
        using IBuffer::IBuffer;

        /// <summary>
        /// Creates an upload buffer.
        /// </summary>
        [[nodiscard]] static IUploadBuffer* Create(
            ISwapchain*       Swapchain,
            const BufferDesc& Desc);

        /// <summary>
        /// Makes the buffer available for reading/writing by the CPU.
        /// </summary>
        [[nodiscard]] virtual uint8_t* Map() = 0;

        /// <summary>
        /// Makes the buffer available for reading/writing by the CPU.
        /// </summary>
        virtual void Unmap() = 0;

        /// <summary>
        /// Maps the buffer to the specified type.
        /// </summary>
        template<typename _Ty>
        [[nodiscard]] _Ty* Map(
            size_t Offset = 0)
        {
            return std::bit_cast<_Ty*>(Map() + Offset);
        }
    };

    //

    class IReadbackBuffer : public virtual IBuffer
    {
    public:
        using IBuffer::IBuffer;

        /// <summary>
        /// Creates a readback buffer.
        /// </summary>
        [[nodiscard]] static IReadbackBuffer* Create(
            ISwapchain*       Swapchain,
            const BufferDesc& Desc);

        /// <summary>
        /// Makes the buffer available for reading by the CPU.
        /// </summary>
        [[nodiscard]] virtual uint8_t* Map() = 0;

        /// <summary>
        /// Makes the buffer available for reading by the CPU.
        /// </summary>
        virtual void Unmap() = 0;
    };

    //

    class ITexture : public virtual IGpuResource
    {
    public:
        using IGpuResource::IGpuResource;

        /// <summary>
        /// Returns the dimensions of the texture.
        /// </summary>
        [[nodiscard]] virtual const Vector3DI& GetDimensions() const = 0;

        /// <summary>
        /// Returns the number of mip levels in the texture.
        /// </summary>
        [[nodiscard]] virtual uint16_t GetMipLevels() const = 0;
    };
} // namespace Neon::RHI