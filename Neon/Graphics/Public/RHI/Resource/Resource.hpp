#pragma once

#include <RHI/Resource/Common.hpp>

namespace Neon
{
    class Vector3D;
}

namespace Neon::RHI
{
    class IGpuResource
    {
    public:
        virtual ~IGpuResource() = default;
    };

    //

    class IBuffer : public IGpuResource
    {
    public:
        /// <summary>
        /// Get the size of the buffer in bytes.
        /// </summary>
        [[nodiscard]] virtual size_t GetSize() const = 0;
    };

    //

    class IUploadBuffer : public virtual IBuffer
    {
    public:
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

    class ITexture : public IGpuResource
    {
    public:
        /// <summary>
        /// Returns the dimensions of the texture.
        /// </summary>
        [[nodiscard]] virtual const Vector3D& GetDimensions() const = 0;

        /// <summary>
        /// Returns the number of mip levels in the texture.
        /// </summary>
        [[nodiscard]] virtual uint8_t GetMipLevels() const = 0;
    };
} // namespace Neon::RHI