#pragma once

#include <RHI/Resource/Views/GenericView.hpp>

namespace Neon::RHI::Views
{
    class RenderTarget : public Generic
    {
    public:
        RenderTarget() = default;
        RenderTarget(
            size_t                    Size,
            IDescriptorHeapAllocator* Allocator) :
            Generic(Size, Allocator)
        {
        }

        /// <summary>
        /// Bind render target view to descriptor heap.
        /// </summary>
        void Bind(
            IGpuResource*  Resource = nullptr,
            const RTVDesc* Desc     = nullptr,
            size_t         Index    = 0)
        {
            auto& Handle = GetHandle();
            Handle.Heap->CreateRenderTargetView(
                Handle.Offset + Index,
                Resource,
                Desc);
        }
    };
} // namespace Neon::RHI::Views