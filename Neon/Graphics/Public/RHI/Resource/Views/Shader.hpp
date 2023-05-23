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
} // namespace Neon::RHI::Views