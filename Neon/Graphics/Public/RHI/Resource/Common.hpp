#pragma once

#include <cstdint>
#include <Core/BitMask.hpp>

namespace Neon::RHI
{
    class IGpuResource;

    struct CpuDescriptorHandle
    {
        uint64_t Value;
    };

    struct GpuDescriptorHandle
    {
        uint64_t Value;
    };

    struct GpuResourceHandle
    {
        uint64_t Value;
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

    //

    enum class EResourceFormat : uint8_t
    {
        Unknown,
        R32G32B32A32_Typeless,
        R32G32B32A32_Float,
        R32G32B32A32_UInt,
        R32G32B32A32_SInt,
        R32G32B32_Typeless,
        R32G32B32_Float,
        R32G32B32_UInt,
        R32G32B32_SInt,
        R16G16B16A16_Typeless,
        R16G16B16A16_Float,
        R16G16B16A16_UNorm,
        R16G16B16A16_UInt,
        R16G16B16A16_SNorm,
        R16G16B16A16_SInt,
        R32G32_Typeless,
        R32G32_Float,
        R32G32_UInt,
        R32G32_SInt,
        R32G8X24_Typeless,
        D32_Float_S8X24_UInt,
        R32_Float_X8X24_Typeless,
        X32_Typeless_G8X24_UInt,
        R10G10B10A2_Typeless,
        R10G10B10A2_UNorm,
        R10G10B10A2_UInt,
        R11G11B10_Float,
        R8G8B8A8_Typeless,
        R8G8B8A8_UNorm,
        R8G8B8A8_UNorm_SRGB,
        R8G8B8A8_UInt,
        R8G8B8A8_SNorm,
        R8G8B8A8_SInt,
        R16G16_Typeless,
        R16G16_Float,
        R16G16_UNorm,
        R16G16_UInt,
        R16G16_SNorm,
        R16G16_SInt,
        R32_Typeless,
        D32_Float,
        R32_Float,
        R32_UInt,
        R32_SInt,
        R24G8_Typeless,
        D24_UNorm_S8_UInt,
        R24_UNorm_X8_Typeless,
        X24_Typeless_G8_UInt,
        R8G8_Typeless,
        R8G8_UNorm,
        R8G8_UInt,
        R8G8_SNorm,
        R8G8_SInt,
        R16_Typeless,
        R16_Float,
        D16_UNorm,
        R16_UNorm,
        R16_UInt,
        R16_SNorm,
        R16_SInt,
        R8_Typeless,
        R8_UNorm,
        R8_UInt,
        R8_SNorm,
        R8_SInt,
        A8_UNorm,
        R1_UNorm,
        R9G9B9E5_SharedExp,
        R8G8_B8G8_UNorm,
        G8R8_G8B8_UNorm,
        BC1_Typeless,
        BC1_UNorm,
        BC1_UNorm_SRGB,
        BC2_Typeless,
        BC2_UNorm,
        BC2_UNorm_SRGB,
        BC3_Typeless,
        BC3_UNorm,
        BC3_UNorm_SRGB,
        BC4_Typeless,
        BC4_UNorm,
        BC4_SNorm,
        BC5_Typeless,
        BC5_UNorm,
        BC5_SNorm,
        B5G6R5_UNorm,
        B5G5R5A1_UNorm,
        B8G8R8A8_UNorm,
        B8G8R8X8_UNorm,
        R10G10B10_XR_Bias_A2_UNorm,
        B8G8R8A8_Typeless,
        B8G8R8A8_UNorm_SRGB,
        B8G8R8X8_Typeless,
        B8G8R8X8_UNorm_SRGB,
        BC6H_Typeless,
        BC6H_UF16,
        BC6H_SF16,
        BC7_Typeless,
        BC7_UNorm,
        BC7_UNorm_SRGB,
        AYUV,
        Y410,
        Y416,
        NV12,
        P010,
        P016,
        Opaque_420,
        YUY2,
        Y210,
        Y216,
        NV11,
        AI44,
        IA44,
        P8,
        A8P8,
        B4G4R4A4_UNorm,
        P208,
        V208,
        V408,
        SamplerFeedback_MinMip_Opaque,
        SamplerFeedback_MipRegionUsed_Opaque,
    };

    //

    enum class ESamplerFilter : uint8_t
    {
        MinMagMipPoint,
        MinMagPoint_MipLinear,
        MinPoint_MagLinear_MipPoint,
        MinPoint_MagMipLinear,
        MinLinear_MagMipPoint,
        MinLinear_MagPoint_MipLinear,
        MinMagLinear_MipPoint,
        MinMagMipLinear,
        Anisotropic,
        Comparison_MinMagMipPoint,
        Comparison_MinMagPoint_MipLinear,
        Comparison_MinPoint_MagLinear_MipPoint,
        Comparison_MinPoint_MagMipLinear,
        Comparison_MinLinear_MagMipPoint,
        Comparison_MinLinear_MagPoint_MipLinear,
        Comparison_MinMagLinear_MipPoint,
        Comparison_MinMagMipLinear,
        Comparison_Anisotropic,
        Minimum_MinMagMipPoint,
        Minimum_MinMagPoint_MipLinear,
        Minimum_MinPoint_MagLinear_MipPoint,
        Minimum_MinPoint_MagMipLinear,
        Minimum_MinLinear_MagMipPoint,
        Minimum_MinLinear_MagPoint_MipLinear,
        Minimum_MinMagLinear_MipPoint,
        Minimum_MinMagMipLinear,
        Minimum_Anisotropic,
        Maximum_MinMagMipPoint,
        Maximum_MinMagPoint_MipLinear,
        Maximum_MinPoint_MagLinear_MipPoint,
        Maximum_MinPoint_MagMipLinear,
        Maximum_MinLinear_MagMipPoint,
        Maximum_MinLinear_MagPoint_MipLinear,
        Maximum_MinMagLinear_MipPoint,
        Maximum_MinMagMipLinear,
        Maximum_Anisotropic
    };

    enum class ESamplerMode : uint8_t
    {
        Wrap,
        Mirror,
        Clamp,
        Border,
        MirrorOnce
    };

    enum class ESamplerCmp : uint8_t
    {
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always
    };

    //

    enum class EResourceFlags : uint8_t
    {
        AllowRenderTarget,
        AllowDepthStencil,
        AllowUnorderedAccess,
        AllowCrossAdapter,
        AllowSimultaneousAccess,
        DenyShaderResource,
        RayTracingAccelerationStruct,

        _Last_Enum
    };
    using MResourceFlags = Bitmask<EResourceFlags>;

    //

    enum class ERootDescriptorFlags : uint8_t
    {
        Descriptor_Volatile,
        Descriptor_Static_Bounds_Check,
        Data_Volatile,
        Data_Static_While_Execute,
        Data_Static,

        _Last_Enum
    };
    using MRootDescriptorFlags = Bitmask<ERootDescriptorFlags>;
} // namespace Neon::RHI