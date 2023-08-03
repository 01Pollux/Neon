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
            Buffer = nullptr;
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
            uint32_t Offset = 0;
            if (!Buffer)
            {
                Buffer.reset(RHI::IUploadBuffer::Create({ .Size = MaxSize }));
                BufferData = Buffer->Map();

                CurSize = 0;
            }

            Offset = CurSize;
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

        [[nodiscard]] auto GetHandleFor(
            uint32_t Offset) const
        {
            return Buffer->GetHandle(CurSize - Offset);
        }
    };
} // namespace Neon::Renderer