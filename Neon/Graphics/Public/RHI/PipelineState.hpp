#pragma once

#include <Core/Neon.hpp>
#include <RHI/Resource/Common.hpp>
#include <variant>
#include <span>

namespace Neon::RHI
{
    class IShader;
    class IRootSignature;

    template<bool _Compute>
    struct PipelineStateBuilder;

    template<>
    struct PipelineStateBuilder<false>
    {
        struct RenderTarget
        {
            bool BlendEnable : 1 = false;
            bool LogicEnable : 1 = false;

            LogicOp OpLogic = LogicOp::Noop;

            BlendTarget Src   = BlendTarget::One;
            BlendTarget Dest  = BlendTarget::Zero;
            BlendOp     OpSrc = BlendOp::Add;

            BlendTarget SrcAlpha  = BlendTarget::One;
            BlendTarget DestAlpha = BlendTarget::Zero;
            BlendOp     OpAlpha   = BlendOp::Add;

            uint8_t WriteMask = 0xF;
        };

        struct BlendState
        {
            bool AlphaToCoverageEnable  = false;
            bool IndependentBlendEnable = false;

            std::array<RenderTarget, 8> RenderTargets;
        };

        struct RasterizerState
        {
            FillMode FillMode              = FillMode::Solid;
            CullMode CullMode              = CullMode::Back;
            bool     FrontCounterClockwise = false;
            int32_t  DepthBias             = 0;
            float    DepthBiasClamp        = 0.0f;
            float    SlopeScaledDepthBias  = 0.0f;
            bool     DepthClipEnable       = true;
            bool     MultisampleEnable     = false;
            bool     AntialiasedLineEnable = false;
            uint32_t ForcedSampleCount     = 0;
            bool     ConservativeRaster    = false;
        };

        struct DepthStencilState
        {
            bool         DepthEnable      = true;
            ECompareFunc DepthCmpFunc     = ECompareFunc::Less;
            bool         DepthWriteEnable = true;

            bool    StencilEnable    = false;
            uint8_t StencilReadMask  = 0xFF;
            uint8_t StencilWriteMask = 0xFF;

            struct
            {
                EStencilOp   FailOp      = EStencilOp::Keep;
                EStencilOp   DepthFailOp = EStencilOp::Keep;
                EStencilOp   PassOp      = EStencilOp::Keep;
                ECompareFunc CmpOp       = ECompareFunc::Always;
            } StencilFrontFace, StencilBackFace;
        };

        enum class StripCutType : uint8_t
        {
            None,
            MaxUInt16,
            MaxUInt32
        };

        enum class PrimitiveTopology : uint8_t
        {
            Undefined,
            Point,
            Line,
            Triangle,
            Patch
        };

        Ptr<IRootSignature> RootSignature = nullptr;

        Ptr<IShader> VertexShader   = nullptr;
        Ptr<IShader> PixelShader    = nullptr;
        Ptr<IShader> GeometryShader = nullptr;
        Ptr<IShader> HullShader     = nullptr;
        Ptr<IShader> DomainShader   = nullptr;

        BlendState Blend;
        uint32_t   SampleMask = 0xFFFFFFFF;

        RasterizerState Rasterizer;

        DepthStencilState DepthStencil;

        ShaderInputLayout Input;
        bool              UseVertexInput = false;

        StripCutType StripCut = StripCutType::None;

        PrimitiveTopology Topology = PrimitiveTopology::Undefined;

        std::vector<EResourceFormat> RTFormats;
        EResourceFormat              DSFormat = EResourceFormat::Unknown;

        uint32_t SampleCount   = 1;
        uint32_t SampleQuality = 0;
    };
    using PipelineStateBuilderG = PipelineStateBuilder<false>;

    template<>
    struct PipelineStateBuilder<true>
    {
        Ptr<IRootSignature> RootSignature = nullptr;
        Ptr<IShader>        ComputeShader = nullptr;
    };
    using PipelineStateBuilderC = PipelineStateBuilder<true>;

    class IPipelineState
    {
    public:
        [[nodiscard]] static Ptr<IPipelineState> Create(
            const PipelineStateBuilderG& Builder);

        [[nodiscard]] static Ptr<IPipelineState> Create(
            const PipelineStateBuilderC& Builder);

        virtual ~IPipelineState() = default;
    };
} // namespace Neon::RHI