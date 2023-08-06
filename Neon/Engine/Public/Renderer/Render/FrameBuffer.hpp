#pragma once

#include <RHI/Resource/Resource.hpp>

namespace Neon::Renderer
{
    struct FrameBuffer
    {
        /// <summary>
        /// Offset of the vertex buffer.
        /// </summary>
        uint32_t CurSize = 0;

        /// <summary>
        /// Maximum number of vertices in the buffer.
        /// </summary>
        const uint32_t MaxSize;

        /// <summary>
        /// Upload buffer to hold data.
        /// </summary>
        UPtr<RHI::IUploadBuffer> Buffer;

        /// <summary>
        /// Pointer to the data in the buffer.
        /// </summary>
        void* BufferData = nullptr;

        FrameBuffer(
            uint32_t MaxSize = 0) :
            MaxSize(MaxSize)
        {
        }

        NEON_CLASS_NO_COPY(FrameBuffer);
        NEON_CLASS_MOVE(FrameBuffer);

        ~FrameBuffer()
        {
            if (Buffer)
            {
                Buffer->Unmap();
            }
        }

        operator bool() const noexcept
        {
            return MaxSize != 0;
        }

        /// <summary>
        /// Reset the buffer.
        /// </summary>
        void Reset()
        {
            Buffer  = nullptr;
            CurSize = 0;
        }

        /// <summary>
        /// Check if the buffer has enough space to allocate.
        /// </summary>
        bool Reserve(
            uint32_t Size)
        {
            return CurSize + Size <= MaxSize;
        }

        /// <summary>
        /// Allocate and return offset of the data.
        /// </summary>
        [[nodiscard]] uint32_t Allocate(
            uint32_t Size)
        {
            uint32_t Offset = CurSize;
            CurSize += Size;
            return Offset;
        }

        /// <summary>
        /// Allocate pair of offseted data and whether was there a reallocation
        /// </summary>
        [[nodiscard]] uint8_t* AllocateData(
            uint32_t Size)
        {
            uint32_t Offset = Allocate(Size);
            return std::bit_cast<uint8_t*>(BufferData) + Offset;
        }

        /// <summary>
        /// Allocate pair of offseted data and whether was there a reallocation
        /// </summary>
        template<typename _Ty>
        [[nodiscard]] _Ty* AllocateData(
            uint32_t Count = 1)
        {
            return std::bit_cast<_Ty*>(AllocateData(sizeof(_Ty) * Count));
        }

        /// <summary>
        /// Get a handle for the data by subtracting the size in bytes from the current size.
        /// </summary>
        [[nodiscard]] auto GetHandleForSize(
            uint32_t Size) const
        {
            return Buffer->GetHandle(CurSize - Size);
        }

        /// <summary>
        /// Get a handle for the data by subtracting the size in bytes from the current size.
        /// </summary>
        [[nodiscard]] auto GetHandle(
            uint32_t Offset) const
        {
            return Buffer->GetHandle(Offset);
        }

        /// <summary>
        /// Map the buffer to system memory.
        /// </summary>
        void Map()
        {
            if (!Buffer)
            {
                Buffer.reset(RHI::IUploadBuffer::Create({ .Size = MaxSize }));
            }
            BufferData = Buffer->Map();
        }

        /// <summary>
        /// Unmap the buffer from system memory.
        /// </summary>
        void Unmap()
        {
            Buffer->Unmap();
            BufferData = nullptr;
        }
    };
} // namespace Neon::Renderer