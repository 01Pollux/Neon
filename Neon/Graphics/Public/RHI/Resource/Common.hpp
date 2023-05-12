#pragma once

#include <cstdint>
#include <Core/BitMask.hpp>

namespace Neon::RHI
{
    struct CpuDescriptorHandle
    {
        uint64_t Handle;
    };

    struct GpuDescriptorHandle
    {
        uint64_t Handle;
    };

    static constexpr uint32_t Resource_AllSubresources = ~0u;

    //

    enum class EResourceState : uint8_t
    {
        Undefined,
        VertexAndConstantBuffer,
        IndexBuffer,
        RenderTarget,
        UnorderedAccess,
        DepthWrite,
        DepthRead,
        NonPixelShaderResource,
        PixelShaderResource,
        StreamOut,
        IndirectArgument,
        CopyDest,
        CopySource,
        ResolveDest,
        ResolveSource,
        RaytracingAccelerationStructure,
        ShadingRateSource,
        Predication,
        VideoDecodeRead,
        VideoDecodeWrite,
        VideoProcessRead,
        VideoProcessWrite,
        VideoEncodeRead,
        VideoEncodeWrite,

        _Last_Enum,

        Vertex         = VertexAndConstantBuffer,
        ConstantBuffer = VertexAndConstantBuffer,
    };
    using MResourceState = Bitmask<EResourceState>;

    static constexpr inline MResourceState MResourceState_Common;
    static constexpr inline MResourceState MResourceState_Present;

    static inline MResourceState MResourceState_AllShaderResource = BitMask_Or(
        EResourceState::NonPixelShaderResource,
        EResourceState::PixelShaderResource);

    static inline MResourceState MResourceState_GenericRead = BitMask_Or(
        EResourceState::VertexAndConstantBuffer,
        EResourceState::IndexBuffer,
        EResourceState::NonPixelShaderResource,
        EResourceState::PixelShaderResource,
        EResourceState::IndirectArgument,
        EResourceState::CopySource);

} // namespace Neon::RHI