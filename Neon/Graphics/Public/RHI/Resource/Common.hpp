#pragma once

#include <cstdint>
#include <Core/BitMask.hpp>
#include <Math/Colors.hpp>
#include <optional>
#include <variant>

namespace Neon::RHI
{
    class IPipelineState;
    class IRootSignature;
    class IGpuResource;
    class IDescriptorHeap;

/// <summary>
/// Rename a root signature.
/// </summary>
#if !NEON_DIST
    void RenameObject(IRootSignature* Object, const wchar_t* Name);
#else
    void RenameObject(IRootSignature*, const wchar_t*)
    {
    }
#endif

/// <summary>
/// Rename a root signature.
/// </summary>
#if !NEON_DIST
    void RenameObject(IPipelineState* PipelienState, const wchar_t* Name);
#else
    void RenameObject(IPipelineState*, const wchar_t*)
    {
    }
#endif

/// <summary>
/// Rename a GPU resource.
/// </summary>
#if !NEON_DIST
    void RenameObject(IGpuResource* Object, const wchar_t* Name);
#else
    void RenameObject(IGpuResource*, const wchar_t*)
    {
    }
#endif

/// <summary>
/// Rename a GPU resource.
/// </summary>
#if !NEON_DIST
    void RenameObject(IDescriptorHeap* Heap, const wchar_t* Name);
#else
    void RenameObject(IDescriptorHeap*, const wchar_t*)
    {
    }
#endif

    //

    enum class GraphicsBufferType : uint8_t
    {
        Default,
        Upload,
        Readback,

        Count
    };

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

    using ShaderInputLayout = std::vector<std::pair<std::string, EResourceFormat>>;

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

    enum class ECompareFunc : uint8_t
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

    enum class ResourceType : uint8_t
    {
        Unknown,
        Buffer,
        Texture1D,
        Texture2D,
        Texture3D
    };

    enum class ResourceLayout : uint8_t
    {
        Unknown,
        RowMajor,
        StandardSwizzle64KB,
        UndefinedSwizzle64KB,
    };

    struct ClearOperation
    {
        struct DepthStencil
        {
            float   Depth;
            uint8_t Stencil;
        };

        RHI::EResourceFormat               Format;
        std::variant<Color4, DepthStencil> Value;

        constexpr auto operator<=>(const ClearOperation&) const noexcept = default;
    };

    using ClearOperationOpt = std::optional<ClearOperation>;

    struct ResourceDesc
    {
        constexpr bool operator==(const ResourceDesc&) const noexcept = default;

        size_t            Width         = 0;
        size_t            Height        = 0;
        uint32_t          Depth         = 0;
        uint32_t          Alignment     = 0;
        uint32_t          MipLevels     = 0;
        uint32_t          SampleCount   = 1;
        uint32_t          SampleQuality = 0;
        MResourceFlags    Flags;
        ResourceType      Type   = ResourceType::Unknown;
        EResourceFormat   Format = EResourceFormat::Unknown;
        ResourceLayout    Layout = ResourceLayout::Unknown;
        ClearOperationOpt ClearValue;

        /// <summary>
        /// Creates a buffer resource description.
        /// </summary>
        static ResourceDesc Buffer(
            size_t               Size,
            uint32_t             Alignment = 0,
            const MResourceFlags Flags     = {})
        {
            return {
                .Width     = Size,
                .Alignment = Alignment,
                .Flags     = Flags,
                .Type      = ResourceType::Buffer,
                .Layout    = ResourceLayout::RowMajor
            };
        }

        /// <summary>
        /// Creates a texture resource description.
        /// </summary>
        static ResourceDesc Tex1D(
            EResourceFormat      Format,
            size_t               Width,
            uint32_t             ArraySize,
            uint32_t             MipLevels,
            uint32_t             SampleCount   = 1,
            uint32_t             SampleQuality = 0,
            const MResourceFlags Flags         = {},
            ResourceLayout       Layout        = ResourceLayout::Unknown,
            uint32_t             Alignment     = 0)
        {
            return {
                .Width         = Width,
                .Depth         = 1,
                .Alignment     = Alignment,
                .MipLevels     = MipLevels,
                .SampleCount   = SampleCount,
                .SampleQuality = SampleQuality,
                .Flags         = Flags,
                .Type          = ResourceType::Texture1D,
                .Format        = Format,
                .Layout        = Layout
            };
        }

        /// <summary>
        /// Creates a texture resource description.
        /// </summary>
        static ResourceDesc Tex2D(
            EResourceFormat      Format,
            size_t               Width,
            size_t               Height,
            uint32_t             ArraySize,
            uint32_t             MipLevels,
            uint32_t             SampleCount   = 1,
            uint32_t             SampleQuality = 0,
            const MResourceFlags Flags         = {},
            ResourceLayout       Layout        = ResourceLayout::Unknown,
            uint32_t             Alignment     = 0)
        {
            return {
                .Width         = Width,
                .Height        = Height,
                .Depth         = 1,
                .Alignment     = Alignment,
                .MipLevels     = MipLevels,
                .SampleCount   = SampleCount,
                .SampleQuality = SampleQuality,
                .Flags         = Flags,
                .Type          = ResourceType::Texture2D,
                .Format        = Format,
                .Layout        = Layout
            };
        }

        /// <summary>
        /// Creates a texture resource description.
        /// </summary>
        static ResourceDesc Tex3D(
            EResourceFormat      Format,
            size_t               Width,
            size_t               Height,
            uint32_t             Depth,
            uint32_t             MipLevels,
            const MResourceFlags Flags     = {},
            ResourceLayout       Layout    = ResourceLayout::Unknown,
            uint32_t             Alignment = 0)
        {
            return {
                .Width     = Width,
                .Height    = Height,
                .Depth     = Depth,
                .Alignment = Alignment,
                .MipLevels = MipLevels,
                .Flags     = Flags,
                .Type      = ResourceType::Texture3D,
                .Format    = Format,
                .Layout    = Layout
            };
        }

    public:
        /// <summary>
        /// Set clear value for render target view
        /// </summary>
        void SetClearValue(
            RHI::EResourceFormat Format,
            const Color4&        Color);

        /// <summary>
        /// Set clear value for depth stencil view
        /// </summary>
        void SetClearValue(
            RHI::EResourceFormat Format,
            float                Depth,
            uint8_t              Stencil);

        /// <summary>
        /// Set clear value for render target view
        /// </summary>
        void SetClearValue(
            const Color4& Color);

        /// <summary>
        /// Set clear value for depth stencil view
        /// </summary>
        void SetClearValue(
            float   Depth,
            uint8_t Stencil);

        /// <summary>
        /// Unset clear value
        /// </summary>
        void UnsetClearValue();
    };

    //

    enum class BlendTarget : uint8_t
    {
        Zero,
        One,
        SrcColor,
        InvSrcColor,
        SrcAlpha,
        InvSrcAlpha,
        DestAlpha,
        InvDestAlpha,
        DestColor,
        InvDestColor,
        SrcAlphaSat,
        BlendFactor,
        InvBlendFactor,
        Src1Color,
        InvSrc1Color,
        Src1Alpha,
        InvSrc1Alpha
    };

    enum class BlendOp : uint8_t
    {
        Add,
        Subtract,
        RevSubtract,
        Min,
        Max
    };

    enum class LogicOp : uint8_t
    {
        Clear,
        Set,
        Copy,
        CopyInverted,
        Noop,
        Invert,
        And,
        Nand,
        Or,
        Nor,
        Xor,
        Equiv,
        AndReverse,
        AndInverted,
        OrReverse,
        OrInverted
    };

    //

    enum class FillMode : uint8_t
    {
        Wireframe,
        Solid
    };

    enum class CullMode : uint8_t
    {
        None,
        Front,
        Back
    };

    //

    enum class EStencilOp : uint8_t
    {
        Keep,
        Zero,
        Replace,
        IncrSat,
        DecrSat,
        Invert,
        Incr,
        Decr
    };

    //

    enum class PrimitiveTopology : uint8_t
    {
        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip,
        LineList_Adj,
        LineStrip_Adj,
        TriangleList_Adj,
        TriangleStrip_Adj,
        PatchList_1,
        PatchList_2,
        PatchList_3,
        PatchList_4,
        PatchList_5,
        PatchList_6,
        PatchList_7,
        PatchList_8,
        PatchList_9,
        PatchList_10,
        PatchList_11,
        PatchList_12,
        PatchList_13,
        PatchList_14,
        PatchList_15,
        PatchList_16,
        PatchList_17,
        PatchList_18,
        PatchList_19,
        PatchList_20,
        PatchList_21,
        PatchList_22,
        PatchList_23,
        PatchList_24,
        PatchList_25,
        PatchList_26,
        PatchList_27,
        PatchList_28,
        PatchList_29,
        PatchList_30,
        PatchList_31,
        PatchList_32
    };
} // namespace Neon::RHI