#pragma once

#include <RHI/Resource/Views/GenericView.hpp>

namespace Neon::RHI::Views
{
    class UnorderedAccess : public Generic
    {
    public:
        using Generic::Generic;

        UnorderedAccess(
            const Generic& Other) :
            Generic(Other)
        {
        }

        /// <summary>
        /// Bind unordered access view to descriptor heap.
        /// </summary>
        void Bind(
            IGpuResource*  Resource = nullptr,
            const UAVDesc* Desc     = nullptr,
            IGpuResource*  Counter  = nullptr,
            uint32_t       Index    = 0)
        {
            auto& Handle = GetHandle();
            Handle.Heap->CreateUnorderedAccessView(
                Handle.Offset + Index,
                Resource,
                Desc,
                Counter);
        }
    };
} // namespace Neon::RHI::Views