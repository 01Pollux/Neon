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
            IGpuResource* Buffer,
            size_t        Stride,
            size_t        Size)
        {
            Append(Buffer->GetHandle(), Stride, Size);
        }

        /// <summary>
        /// Append a vertex buffer view.
        /// </summary>
        void Append(
            IGpuResource* Buffer,
            size_t        Offset,
            size_t        Stride,
            size_t        Size)
        {
            Append(Buffer->GetHandle(), Offset, Stride, Size);
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
        void Append(
            GpuResourceHandle Handle,
            size_t            Offset,
            size_t            Stride,
            size_t            Size)
        {
            m_Views.emplace_back(GpuResourceHandle{ Handle.Value + Offset * Stride }, uint32_t(Stride), uint32_t(Size));
        }

        /// <summary>
        /// Append a vertex buffer view.
        /// </summary>
        template<typename _Ty>
        void Append(
            GpuResourceHandle BaseHandle,
            uint32_t          Count)
        {
            Append(BaseHandle, sizeof(_Ty), Count * sizeof(_Ty));
        }

        /// <summary>
        /// Append a vertex buffer view.
        /// </summary>
        template<typename _Ty>
        void Append(
            GpuResourceHandle BaseHandle,
            uint32_t          Offset,
            uint32_t          Count)
        {
            Append(BaseHandle, Offset, sizeof(_Ty), Count * sizeof(_Ty));
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

        constexpr Index(
            IGpuResource* Buffer,
            size_t        Size,
            bool          Is32Bit = false) :
            Index(Buffer->GetHandle(), Size, Is32Bit)
        {
        }

        constexpr Index(
            IGpuResource* Buffer,
            size_t        Offset,
            size_t        Size,
            bool          Is32Bit = false) :
            Index(Buffer->GetHandle(), Offset, Size, Is32Bit)
        {
        }

        constexpr Index(
            GpuResourceHandle Handle,
            size_t            Size,
            bool              Is32Bit = false) :
            m_View{ Handle, uint32_t(Size), Is32Bit }
        {
        }

        constexpr Index(
            GpuResourceHandle Handle,
            size_t            Offset,
            size_t            Size,
            bool              Is32Bit = false) :
            m_View{ { Handle.Value + Offset * (Is32Bit ? sizeof(uint32_t) : sizeof(uint16_t)) }, uint32_t(Size), Is32Bit }
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
        constexpr IndexU16() = default;

        constexpr IndexU16(
            IGpuResource* Buffer,
            uint32_t      Count) :
            Index(Buffer, Count * sizeof(uint16_t), false)
        {
        }

        constexpr IndexU16(
            IGpuResource* Buffer,
            uint32_t      Offset,
            uint32_t      Count) :
            Index(Buffer, Offset * sizeof(uint16_t), Count * sizeof(uint16_t), false)
        {
        }

        constexpr IndexU16(
            GpuResourceHandle Handle,
            uint32_t          Count) :
            Index(Handle, Count * sizeof(uint16_t), false)
        {
        }

        constexpr IndexU16(
            GpuResourceHandle Handle,
            uint32_t          Offset,
            uint32_t          Count) :
            Index(Handle, Offset * sizeof(uint16_t), Count * sizeof(uint16_t), false)
        {
        }
    };

    struct IndexU32 : public Index
    {
        constexpr IndexU32() = default;

        constexpr IndexU32(
            IGpuResource* Buffer,
            uint32_t      Count) :
            Index(Buffer, Count * sizeof(uint32_t), true)
        {
        }
        constexpr IndexU32(
            IGpuResource* Buffer,
            uint32_t      Offset,
            uint32_t      Count) :
            Index(Buffer, Offset * sizeof(uint32_t), Count * sizeof(uint32_t), true)
        {
        }

        constexpr IndexU32(
            GpuResourceHandle Handle,
            uint32_t          Count) :
            Index(Handle, Count * sizeof(uint32_t), true)
        {
        }

        constexpr IndexU32(
            GpuResourceHandle Handle,
            uint32_t          Offset,
            uint32_t          Count) :
            Index(Handle, Offset * sizeof(uint32_t), Count * sizeof(uint32_t), true)
        {
        }
    };
} // namespace Neon::RHI::Views