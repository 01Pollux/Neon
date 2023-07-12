#pragma once

#include <RHI/Resource/Views/GenericView.hpp>

namespace Neon::RHI::Views
{
    class Sampler : public Generic
    {
    public:
        using Generic::Generic;

        Sampler(
            const Generic& Other) :
            Generic(Other)
        {
        }

        /// <summary>
        /// Bind render target view to descriptor heap.
        /// </summary>
        void Bind(
            const SamplerDesc& Desc,
            uint32_t           Index = 0)
        {
            auto& Handle = GetHandle();
            Handle.Heap->CreateSampler(
                Handle.Offset + Index,
                Desc);
        }
    };
} // namespace Neon::RHI::Views