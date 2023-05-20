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
            size_t                    Size,
            IDescriptorHeapAllocator* Allocator) :
            m_Allocator(Allocator),
            m_Handle(Allocator->Allocate(Size))
        {
        }

        NEON_CLASS_NO_COPY(Generic);

        Generic(
            Generic&& Other) noexcept :
            m_Allocator(std::exchange(Other.m_Allocator, nullptr)),
            m_Handle(Other.m_Handle)
        {
        }

        Generic& operator=(
            Generic&& Other) noexcept
        {
            if (this != &Other)
            {
                m_Allocator = std::exchange(Other.m_Allocator, nullptr);
                m_Handle    = Other.m_Handle;
            }
            return *this;
        }

        ~Generic()
        {
            if (m_Allocator)
            {
                m_Allocator->Free(m_Handle);
            }
        }

        constexpr bool operator==(
            const Generic& Other) const noexcept = default;

        /// <summary>
        /// Get the CPU descriptor handle.
        /// </summary>
        [[nodiscard]] CpuDescriptorHandle GetCpuHandle(
            size_t Offset = 0) const noexcept
        {
            return m_Handle.Heap->GetCPUAddress(m_Handle.Offset + Offset);
        }

        /// <summary>
        /// Get the GPU descriptor handle.
        /// </summary>
        [[nodiscard]] GpuDescriptorHandle GetGpuHandle(
            size_t Offset = 0) const noexcept
        {
            return m_Handle.Heap->GetGPUAddress(m_Handle.Offset + Offset);
        }

        /// <summary>
        /// Get the descriptor heap allocator.
        /// </summary>
        IDescriptorHeapAllocator* GetAllocator() const noexcept
        {
            return m_Allocator;
        }

        /// <summary>
        /// Get the descriptor handle.
        /// </summary>
        const DescriptorHeapHandle& GetHandle() const noexcept
        {
            return m_Handle;
        }

    private:
        IDescriptorHeapAllocator* m_Allocator = nullptr;
        DescriptorHeapHandle      m_Handle;
    };
} // namespace Neon::RHI::Views