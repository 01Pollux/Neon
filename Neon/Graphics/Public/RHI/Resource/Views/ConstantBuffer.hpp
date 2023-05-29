#pragma once

#include <RHI/Resource/Views/GenericView.hpp>

namespace Neon::RHI::Views
{
    class ConstantBuffer : public Generic
    {
    public:
        using Generic::Generic;

        /// <summary>
        /// Bind constant buffer view to descriptor heap.
        /// </summary>
        void Bind(
            const CBVDesc& Desc,
            uint32_t       Index = 0)
        {
            auto& Handle = GetHandle();
            Handle.Heap->CreateConstantBufferView(
                Handle.Offset + Index,
                Desc);
        }
    };
} // namespace Neon::RHI::Views