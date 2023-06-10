#pragma once

#include <RHI/Resource/Views/GenericView.hpp>

namespace Neon::RHI::Views
{
    class DepthStencil : public Generic
    {
    public:
        using Generic::Generic;

        /// <summary>
        /// Bind render target view to descriptor heap.
        /// </summary>
        void Bind(
            IGpuResource*  Resource = nullptr,
            const DSVDesc* Desc     = nullptr,
            uint32_t       Index    = 0)
        {
            auto& Handle = GetHandle();
            Handle.Heap->CreateDepthStencilView(
                Handle.Offset + Index,
                Resource,
                Desc);
        }
    };
} // namespace Neon::RHI::Views