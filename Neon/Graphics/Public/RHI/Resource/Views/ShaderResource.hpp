#pragma once

#include <RHI/Resource/Views/GenericView.hpp>

namespace Neon::RHI::Views
{
    class ShaderResource : public Generic
    {
    public:
        using Generic::Generic;

        ShaderResource(
            const Generic& Other) :
            Generic(Other)
        {
        }

        /// <summary>
        /// Bind shader resource view to descriptor heap.
        /// </summary>
        void Bind(
            IGpuResource*  Resource = nullptr,
            const SRVDesc* Desc     = nullptr,
            uint32_t       Index    = 0)
        {
            auto& Handle = GetHandle();
            Handle.Heap->CreateShaderResourceView(
                Handle.Offset + Index,
                Resource,
                Desc);
        }
    };
} // namespace Neon::RHI::Views