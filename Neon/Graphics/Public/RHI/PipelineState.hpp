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

    struct PipelineStateBuilder
    {
        struct RenderTarget
        {
            struct LogicBlend
            {
                LogicOp Op = LogicOp::Noop;
            };

            struct ColorBlend
            {
                BlendTarget SrcBlend  = BlendTarget::One;
                BlendTarget DestBlend = BlendTarget::Zero;
                BlendOp     Op        = BlendOp::Add;

                BlendTarget SrcBlendAlpha  = BlendTarget::One;
                BlendTarget DestBlendAlpha = BlendTarget::Zero;
                BlendOp     OpAlpha        = BlendOp::Add;
            };

            std::variant<LogicBlend, ColorBlend> Blend;

            uint8_t WriteMask = 0xF;
        };

        struct BlendState
        {
            bool         AlphaToCoverageEnable  = false;
            bool         IndependentBlendEnable = false;
            RenderTarget RenderTarget[8];
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

        class DepthStencilState
        {
            bool             DepthEnable      = true;
            EDepthStencilCmp DepthCmpFunc     = EDepthStencilCmp::Less;
            bool             DepthWriteEnable = true;

            bool    StencilEnable    = false;
            uint8_t StencilReadMask  = 0xFF;
            uint8_t StencilWriteMask = 0xFF;

            struct
            {
                EStencilOp       FailOp      = EStencilOp::Keep;
                EStencilOp       DepthFailOp = EStencilOp::Keep;
                EStencilOp       PassOp      = EStencilOp::Keep;
                EDepthStencilCmp CmpOp       = EDepthStencilCmp::Always;
            } StencilFrontFace, StencilBackFace;
        };

        enum class StripCutType : uint8_t
        {
            None,
            MaxUInt16,
            MaxUInt32
        };

        enum class PrimitiveTopologyType : uint8_t
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

        PrimitiveTopologyType PrimitiveTopology = PrimitiveTopologyType::Undefined;

        std::vector<EResourceFormat> RTFormats;
        EResourceFormat              DSFormat = EResourceFormat::Unknown;

        uint32_t SampleCount   = 1;
        uint32_t SampleQuality = 0;
    };

    struct ComputePipelineStateBuilder
    {
        IRootSignature* RootSignature = nullptr;
        IShader*        ComputeShader = nullptr;
    };

    class IPipelineState
    {
    public:
        [[nodiscard]] Ptr<IPipelineState> Create(
            const PipelineStateBuilder& Builder);

        [[nodiscard]] Ptr<IPipelineState> Create(
            const ComputePipelineStateBuilder& Builder);

        virtual ~IPipelineState() = default;
    };
} // namespace Neon::RHI