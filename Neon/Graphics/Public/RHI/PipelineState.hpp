#pragma once

#include <Core/Neon.hpp>
#include <RHI/Resource/Common.hpp>
#include <RHI/Resource/MappedBuffer.hpp>
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
            bool         AlphaToCoverageEnable  = false;
            bool         IndependentBlendEnable = false;
            RenderTarget RenderTargets[8];
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

        enum class Toplogy : uint8_t
        {
            Undefined,
            Point,
            Line,
            Triangle,
            Patch
        };

        IRootSignature* RootSignature = nullptr;

        IShader* VertexShader   = nullptr;
        IShader* PixelShader    = nullptr;
        IShader* GeometryShader = nullptr;
        IShader* HullShader     = nullptr;
        IShader* DomainShader   = nullptr;

        BlendState BlendState;
        uint32_t   SampleMask = 0xFFFFFFFF;

        RasterizerState RasterizerState;

        DepthStencilState DepthStencilState;

        MBuffer::RawLayout InputLayout;

        StripCutType StripCut = StripCutType::None;

        Toplogy PrimitiveTopology = Toplogy::Undefined;

        std::vector<EResourceFormat> RTFormats;
        EResourceFormat              DSFormat = EResourceFormat::Unknown;

        uint32_t SampleCount   = 1;
        uint32_t SampleQuality = 0;
    };

    template<>
    struct PipelineStateBuilder<true>
    {
        IRootSignature* RootSignature = nullptr;
        IShader*        ComputeShader = nullptr;
    };

    class IPipelineState
    {
    public:
        [[nodiscard]] static Ptr<IPipelineState> Create(
            const PipelineStateBuilder<false>& Builder);

        [[nodiscard]] static Ptr<IPipelineState> Create(
            const PipelineStateBuilder<true>& Builder);

        virtual ~IPipelineState() = default;
    };
} // namespace Neon::RHI