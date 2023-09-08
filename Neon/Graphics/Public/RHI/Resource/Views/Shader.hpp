#pragma once

#include <RHI/Resource/Resource.hpp>
#include <vector>

namespace Neon::RHI::Views
{
    class Vertex
    {
    public:
        struct View
        {
            GpuResourceHandle Handle;
            uint32_t          Stride;
            uint32_t          Size;
        };

        Vertex() = default;

        /// <summary>
        /// Append a vertex buffer view.
        /// </summary>
        void Append(
            IBuffer* Buffer,
            size_t   Offset,
            size_t   Stride,
            size_t   Size)
        {
            Append({ Buffer->GetHandle().Value + Offset }, Stride, Size);
        }

        /// <summary>
        /// Append a vertex buffer view.
        /// </summary>
        void Append(
            GpuResourceHandle Handle,
            size_t            Stride,
            size_t            Size)
        {
            m_Views.emplace_back(Handle, uint32_t(Stride), uint32_t(Size));
        }

        /// <summary>
        /// Append a vertex buffer view.
        /// </summary>
        template<typename _Ty>
        void AppendOffset(
            GpuResourceHandle BaseHandle,
            uint32_t          Offset,
            uint32_t          Count)
        {
            Append({ BaseHandle.Value + Offset * sizeof(_Ty) }, sizeof(_Ty), sizeof(_Ty) * Count);
        }

        /// <summary>
        /// Get the views.
        /// </summary>
        [[nodiscard]] auto& GetViews() const noexcept
        {
            return m_Views;
        }

    private:
        std::vector<View> m_Views;
    };

    class Index
    {
    public:
        struct View
        {
            GpuResourceHandle Handle{};
            uint32_t          Size{};
            bool              Is32Bit = false;
        };

        Index() = default;

        Index(
            IBuffer* Buffer,
            size_t   Offset,
            size_t   Size,
            bool     Is32Bit = false) :
            Index({ Buffer->GetHandle().Value + Offset }, Size, Is32Bit)
        {
        }

        Index(
            GpuResourceHandle Handle,
            size_t            Size,
            bool              Is32Bit = false) :
            m_View{ Handle, uint32_t(Size), Is32Bit }
        {
        }

        /// <summary>
        /// Get the views.
        /// </summary>
        [[nodiscard]] auto& Get() const noexcept
        {
            return m_View;
        }

    private:
        View m_View;
    };

    struct IndexU16 : public Index
    {
        IndexU16() = default;

        IndexU16(
            IBuffer* Buffer,
            uint32_t Offset,
            uint32_t Count) :
            Index(Buffer, Offset * sizeof(uint32_t), Count * sizeof(uint16_t), false)
        {
        }

        IndexU16(
            GpuResourceHandle Handle,
            uint32_t          Count) :
            Index(Handle, Count * sizeof(uint16_t), false)
        {
        }
    };

    struct IndexU32 : public Index
    {
        IndexU32() = default;

        IndexU32(
            IBuffer* Buffer,
            uint32_t Offset,
            uint32_t Count) :
            Index(Buffer, Offset * sizeof(uint32_t), Count * sizeof(uint32_t), true)
        {
        }

        IndexU32(
            GpuResourceHandle Handle,
            uint32_t          Count) :
            Index(Handle, Count * sizeof(uint32_t), true)
        {
        }
    };
} // namespace Neon::RHI::Views