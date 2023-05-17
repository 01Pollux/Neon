#pragma once

#include <RHI/Resource/Common.hpp>

namespace Neon
{
    class Vector3DI;
}

namespace Neon::RHI
{
    class ISwapchain;

    class IGpuResource
    {
    public:
        IGpuResource(
            ISwapchain* Swapchain);
        NEON_CLASS_COPYMOVE(IGpuResource);
        virtual ~IGpuResource() = default;

    protected:
        ISwapchain* m_OwningSwapchain = nullptr;
    };

    //

    enum class BufferUsage
    {
        VertexBuffer,
        IndexBuffer,
        ConstantBuffer,
        StructuredBuffer,
        RawBuffer,
        IndirectBuffer,
        AccelerationStructure,
    };

    class IBuffer : public virtual IGpuResource
    {
    public:
        struct Desc
        {
            size_t      Size;
            uint32_t    Alignment;
            BufferUsage Usage;
        };

        using IGpuResource::IGpuResource;

        /// <summary>
        /// Creates a buffer.
        /// </summary>
        [[nodiscard]] static IBuffer* Create(
            ISwapchain* Swapchain,
            const Desc& Desc);

        /// <summary>
        /// Get the size of the buffer in bytes.
        /// </summary>
        [[nodiscard]] virtual size_t GetSize() const = 0;
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
            ISwapchain* Swapchain,
            const Desc& Desc);

        /// <summary>
        /// Makes the buffer available for reading/writing by the CPU.
        /// </summary>
        [[nodiscard]] virtual uint8_t* Map() = 0;

        /// <summary>
        /// Makes the buffer available for reading/writing by the CPU.
        /// </summary>
        virtual void Unmap() = 0;
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
            ISwapchain* Swapchain,
            const Desc& Desc);

        /// <summary>
        /// Makes the buffer available for reading by the CPU.
        /// </summary>
        [[nodiscard]] virtual const uint8_t* Map() = 0;

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