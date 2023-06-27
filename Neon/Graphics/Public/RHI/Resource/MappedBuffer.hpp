#pragma once

#include <Core/Neon.hpp>
#include <Utils/Struct.hpp>
#include <RHI/Resource/Resource.hpp>

namespace Neon::RHI
{
    class GPUBuffer
    {
    public:
        enum class Type
        {
            Default,
            Upload,
            Readback
        };

        struct Desc : BufferDesc
        {
            ISwapchain* Swapchain;
            Type        BufferType;
        };

        static constexpr size_t CBufferAlignement = 256;

        GPUBuffer() = default;

        /// <summary>
        /// Reference an existing buffer with a struct layout
        /// </summary>
        GPUBuffer(
            const Structured::CookedLayout* Layout,
            const Ptr<IBuffer>&             Buffer,
            Type                            BufferType);

        /// <summary>
        /// Create a new buffer with a struct layout
        /// </summary>
        GPUBuffer(
            const Structured::CookedLayout* Layout,
            const Desc&                     BufferDesc);

        /// <summary>
        /// Create a new buffer with a struct layout
        /// </summary>
        GPUBuffer(
            const Structured::RawLayout& Layout,
            const Desc&                  BufferDesc);

        NEON_CLASS_COPYMOVE(GPUBuffer);

        ~GPUBuffer();

        /// <summary>
        /// Get element in struct layout
        /// </summary>
        [[nodiscard]] auto operator[](
            const std::string& Name)
        {
            return Structured::BufferView(static_cast<uint8_t*>(m_MappedData), (*m_Layout)[Name], 0);
        }

        /// <summary>
        /// Get element in struct layout
        /// </summary>
        [[nodiscard]] auto operator[](
            const std::string& Name) const
        {
            return Structured::CBufferView(static_cast<const uint8_t*>(m_MappedData), (*m_Layout)[Name], 0);
        }

        /// <summary>
        /// Get GPU buffer
        /// </summary>
        [[nodiscard]] const Ptr<IBuffer>& GetBuffer() const
        {
            return m_Buffer;
        }

        /// <summary>
        /// Get cooked layout for buffer
        /// </summary>
        [[nodiscard]] const Structured::CookedLayout* GetLayout() const noexcept
        {
            return m_Layout;
        }

    private:
        /// <summary>
        /// Map buffer to CPU
        /// </summary>
        void Map();

        /// <summary>
        /// Unmap buffer from CPU
        /// </summary>
        void Unmap();

    private:
        const Structured::CookedLayout* m_Layout = nullptr;

        Ptr<IBuffer> m_Buffer;
        uint8_t*     m_MappedData = nullptr;
        Type         m_BufferType;
    };
} // namespace Neon::RHI
