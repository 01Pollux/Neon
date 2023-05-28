#pragma once

#include <RHI/Resource/Descriptor.hpp>
#include <RHI/Resource/View.hpp>

namespace Neon::RHI::Views
{
    class Generic
    {
    public:
        Generic() = default;
        Generic(
            IDescriptorHeapAllocator* Allocator,
            uint32_t                  Size) :
            m_Handle(Allocator->Allocate(Size))
        {
        }

        Generic(
            const DescriptorHeapHandle& Handle) :
            m_Handle(Handle)
        {
        }

        constexpr bool operator==(
            const Generic& Other) const noexcept = default;

        operator bool() const noexcept
        {
            return static_cast<bool>(m_Handle);
        }

        /// <summary>
        /// Get the CPU descriptor handle.
        /// </summary>
        [[nodiscard]] CpuDescriptorHandle GetCpuHandle(
            uint32_t Offset = 0) const noexcept
        {
            return m_Handle.Heap->GetCPUAddress(m_Handle.Offset + Offset);
        }

        /// <summary>
        /// Get the GPU descriptor handle.
        /// </summary>
        [[nodiscard]] GpuDescriptorHandle GetGpuHandle(
            uint32_t Offset = 0) const noexcept
        {
            return m_Handle.Heap->GetGPUAddress(m_Handle.Offset + Offset);
        }

        /// <summary>
        /// Get the descriptor handle.
        /// </summary>
        const DescriptorHeapHandle& GetHandle() const noexcept
        {
            return m_Handle;
        }

    private:
        DescriptorHeapHandle m_Handle;
    };
} // namespace Neon::RHI::Views