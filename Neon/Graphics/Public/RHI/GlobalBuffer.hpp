#pragma once

#include <Core/Neon.hpp>
#include <RHI/Resource/Resource.hpp>

namespace Neon::RHI
{
    /// <summary>
    /// A global buffer pool is a buffer pool that is persistent and can be used by multiple threads
    /// It is used for copying date from and to the GPU
    /// </summary>
    class IGlobalBufferPool
    {
    public:
        enum class BufferType : uint8_t
        {
            ReadOnly,
            ReadWrite,
            ReadWriteGPUR,
            ReadWriteGPURW,
            Count
        };

        struct Handle
        {
            GpuBuffer Buffer;

            size_t Offset;
            size_t Size;

            BufferType Type;

            /// <summary>
            /// Get the GPU handle
            /// </summary>
            [[nodiscard]] RHI::GpuResourceHandle GetGpuHandle(
                size_t RelOffset = 0) const
            {
                return Buffer.GetHandle(Offset + RelOffset);
            }

            Handle() = default;

            Handle(
                size_t     Size,
                size_t     Alignement,
                BufferType Type);

            [[nodiscard]] operator bool() const noexcept
            {
                return Buffer != nullptr;
            }

            /// <summary>
            /// Cast to an upload buffer
            /// </summary>
            /// <returns></returns>
            [[nodiscard]] GpuUploadBuffer AsUpload() const noexcept
            {
                return GpuUploadBuffer(Buffer);
            }

            /// <summary>
            /// Cast to a readback buffer
            /// </summary>
            /// <returns></returns>
            [[nodiscard]] GpuReadbackBuffer AsReadBack() const noexcept
            {
                return GpuReadbackBuffer(Buffer);
            }
        };

        /// <summary>
        /// Allocate a buffer
        /// </summary>
        [[nodiscard]] static Handle Allocate(
            size_t     Size,
            size_t     Alignement,
            BufferType Type);

        /// <summary>
        /// Free buffer handles
        /// </summary>
        static void Free(
            std::span<const Handle> Handles);

        /// <summary>
        /// Free a buffer handle
        /// </summary>
        static void Free(
            const Handle& Handle)
        {
            Free({ &Handle, 1 });
        }
    };

    using BufferPoolHandle = IGlobalBufferPool::Handle;

    inline IGlobalBufferPool::Handle::Handle(
        size_t                        Size,
        size_t                        Alignement,
        IGlobalBufferPool::BufferType Type)
    {
        *this = IGlobalBufferPool::Allocate(Size, Alignement, Type);
    }

    /// <summary>
    /// Safe buffer pool handle that frees the handle when it goes out of scope
    /// </summary>
    struct UBufferPoolHandle : BufferPoolHandle
    {
        using BufferPoolHandle::BufferPoolHandle;

        UBufferPoolHandle(
            BufferPoolHandle&& Other) noexcept :
            BufferPoolHandle(std::move(Other))
        {
        }

        NEON_CLASS_NO_COPY(UBufferPoolHandle);

        UBufferPoolHandle(
            UBufferPoolHandle&& Other) noexcept :
            BufferPoolHandle(std::move(Other))
        {
            Other.Buffer = nullptr;
        }

        UBufferPoolHandle& operator=(
            UBufferPoolHandle&& Other) noexcept
        {
            if (this != &Other)
            {
                UBufferPoolHandle::~UBufferPoolHandle();
                BufferPoolHandle::operator=(std::move(Other));
                Other.Buffer = nullptr;
            }

            return *this;
        }

        ~UBufferPoolHandle()
        {
            if (Buffer)
            {
                IGlobalBufferPool::Free(*this);
            }
        }
    };
} // namespace Neon::RHI