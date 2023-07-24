#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Resource/Common.hpp>
#include <Private/RHI/Dx12/PipelineState.hpp>
#include <Private/RHI/Dx12/Device.hpp>
#include <Private/RHI/Dx12/RootSignature.hpp>
#include <RHI/Shader.hpp>

namespace views  = std::views;
namespace ranges = std::ranges;

namespace Neon::RHI
{
    std::mutex                                           s_PipelineStateCacheMutex;
    std::map<Crypto::Sha256::Bytes, Ptr<IPipelineState>> s_PipelineStateCache;

    //

    /// <summary>
    /// Casts a Neon blend target to a D3D12 blend target.
    /// </summary>
    [[nodiscard]] D3D12_BLEND CastBlendTarget(
        BlendTarget Target)
    {
        switch (Target)
        {
        case BlendTarget::Zero:
            return D3D12_BLEND_ZERO;
        case BlendTarget::One:
            return D3D12_BLEND_ONE;
        case BlendTarget::SrcColor:
            return D3D12_BLEND_SRC_COLOR;
        case BlendTarget::InvSrcColor:
            return D3D12_BLEND_INV_SRC_COLOR;
        case BlendTarget::SrcAlpha:
            return D3D12_BLEND_SRC_ALPHA;
        case BlendTarget::InvSrcAlpha:
            return D3D12_BLEND_INV_SRC_ALPHA;
        case BlendTarget::DestAlpha:
            return D3D12_BLEND_DEST_ALPHA;
        case BlendTarget::InvDestAlpha:
            return D3D12_BLEND_INV_DEST_ALPHA;
        case BlendTarget::DestColor:
            return D3D12_BLEND_DEST_COLOR;
        case BlendTarget::InvDestColor:
            return D3D12_BLEND_INV_DEST_COLOR;
        case BlendTarget::SrcAlphaSat:
            return D3D12_BLEND_SRC_ALPHA_SAT;
        case BlendTarget::BlendFactor:
            return D3D12_BLEND_BLEND_FACTOR;
        case BlendTarget::InvBlendFactor:
            return D3D12_BLEND_INV_BLEND_FACTOR;
        case BlendTarget::Src1Color:
            return D3D12_BLEND_SRC1_COLOR;
        case BlendTarget::InvSrc1Color:
            return D3D12_BLEND_INV_SRC1_COLOR;
        case BlendTarget::Src1Alpha:
            return D3D12_BLEND_SRC1_ALPHA;
        case BlendTarget::InvSrc1Alpha:
            return D3D12_BLEND_INV_SRC1_ALPHA;
        default:
            std::unreachable();
        }
    }

    /// <summary>
    /// Casts a Neon blend operation to a D3D12 blend operation.
    /// </summary>
    [[nodiscard]] D3D12_BLEND_OP CastBlendOp(
        BlendOp Op)
    {
        switch (Op)
        {
        case BlendOp::Add:
            return D3D12_BLEND_OP_ADD;
        case BlendOp::Subtract:
            return D3D12_BLEND_OP_SUBTRACT;
        case BlendOp::RevSubtract:
            return D3D12_BLEND_OP_REV_SUBTRACT;
        case BlendOp::Min:
            return D3D12_BLEND_OP_MIN;
        case BlendOp::Max:
            return D3D12_BLEND_OP_MAX;
        default:
            std::unreachable();
        }
    }

    /// <summary>
    /// Casts a Neon logic operation to a D3D12 logic operation.
    /// </summary>
    [[nodiscard]] D3D12_LOGIC_OP CastLogicOp(
        LogicOp Op)
    {
        switch (Op)
        {
        case LogicOp::Clear:
            return D3D12_LOGIC_OP_CLEAR;
        case LogicOp::Set:
            return D3D12_LOGIC_OP_SET;
        case LogicOp::Copy:
            return D3D12_LOGIC_OP_COPY;
        case LogicOp::CopyInverted:
            return D3D12_LOGIC_OP_COPY_INVERTED;
        case LogicOp::Noop:
            return D3D12_LOGIC_OP_NOOP;
        case LogicOp::Invert:
            return D3D12_LOGIC_OP_INVERT;
        case LogicOp::And:
            return D3D12_LOGIC_OP_AND;
        case LogicOp::Nand:
            return D3D12_LOGIC_OP_NAND;
        case LogicOp::Or:
            return D3D12_LOGIC_OP_OR;
        case LogicOp::Nor:
            return D3D12_LOGIC_OP_NOR;
        case LogicOp::Xor:
            return D3D12_LOGIC_OP_XOR;
        case LogicOp::Equiv:
            return D3D12_LOGIC_OP_EQUIV;
        case LogicOp::AndReverse:
            return D3D12_LOGIC_OP_AND_REVERSE;
        case LogicOp::AndInverted:
            return D3D12_LOGIC_OP_AND_INVERTED;
        case LogicOp::OrReverse:
            return D3D12_LOGIC_OP_OR_REVERSE;
        case LogicOp::OrInverted:
            return D3D12_LOGIC_OP_OR_INVERTED;
        default:
            std::unreachable();
        }
    }

    /// <summary>
    /// Casts a Neon fill mode to a D3D12 fill mode.
    ///
    [[nodiscard]] D3D12_FILL_MODE CastFillMode(
        FillMode Mode)
    {
        switch (Mode)
        {
        case FillMode::Wireframe:
            return D3D12_FILL_MODE_WIREFRAME;
        case FillMode::Solid:
            return D3D12_FILL_MODE_SOLID;
        default:
            std::unreachable();
        }
    }

    /// <summary>
    /// Casts a Neon cull mode to a D3D12 cull mode.
    ///
    [[nodiscard]] D3D12_CULL_MODE CastCullMode(
        ECullMode Mode)
    {
        switch (Mode)
        {
        case ECullMode::None:
            return D3D12_CULL_MODE_NONE;
        case ECullMode::Front:
            return D3D12_CULL_MODE_FRONT;
        case ECullMode::Back:
            return D3D12_CULL_MODE_BACK;
        default:
            std::unreachable();
        }
    }

    /// <summary>
    /// Casts a Neon stencil operation to a D3D12 stencil operation.
    /// </summary>
    [[nodiscard]] D3D12_STENCIL_OP CastStencilOp(
        EStencilOp Op)
    {
        switch (Op)
        {
        case EStencilOp::Keep:
            return D3D12_STENCIL_OP_KEEP;
        case EStencilOp::Zero:
            return D3D12_STENCIL_OP_ZERO;
        case EStencilOp::Replace:
            return D3D12_STENCIL_OP_REPLACE;
        case EStencilOp::IncrSat:
            return D3D12_STENCIL_OP_INCR_SAT;
        case EStencilOp::DecrSat:
            return D3D12_STENCIL_OP_DECR_SAT;
        case EStencilOp::Invert:
            return D3D12_STENCIL_OP_INVERT;
        case EStencilOp::Incr:
            return D3D12_STENCIL_OP_INCR;
        case EStencilOp::Decr:
            return D3D12_STENCIL_OP_DECR;
        default:
            std::unreachable();
        }
    }

    //

    Ptr<IPipelineState> IPipelineState::Create(
        const PipelineStateBuilderG& Builder)
    {
        return Dx12PipelineStateCache::Load(Builder);
    }

    Ptr<IPipelineState> IPipelineState::Create(
        const PipelineStateBuilderC& Builder)
    {
        return Dx12PipelineStateCache::Load(Builder);
    }

    //

    Dx12PipelineState::Dx12PipelineState(
        const D3D12_GRAPHICS_PIPELINE_STATE_DESC& GraphicsDesc)
    {
        auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();
        ThrowIfFailed(Dx12Device->CreateGraphicsPipelineState(
            &GraphicsDesc,
            IID_PPV_ARGS(&m_PipelineState)));
    }

    Dx12PipelineState::Dx12PipelineState(
        const D3D12_COMPUTE_PIPELINE_STATE_DESC& ComputeDesc)
    {
        auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();
        ThrowIfFailed(Dx12Device->CreateComputePipelineState(
            &ComputeDesc,
            IID_PPV_ARGS(&m_PipelineState)));
    }

    ID3D12PipelineState* Dx12PipelineState::Get()
    {
        return m_PipelineState.Get();
    }

    //

    void Dx12PipelineStateCache::Flush()
    {
        std::scoped_lock Lock(s_PipelineStateCacheMutex);
        s_PipelineStateCache.clear();
    }

    Ptr<IPipelineState> Dx12PipelineStateCache::Load(
        const PipelineStateBuilderG& Builder)
    {
        auto Result = Dx12PipelineStateCache::Build(Builder);

        std::scoped_lock Lock(s_PipelineStateCacheMutex);

        auto& Cache = s_PipelineStateCache[Result.Digest];
        if (!Cache)
        {
            Cache = std::make_shared<Dx12PipelineState>(Result.Desc);
        }

        return Cache;
    }

    Ptr<IPipelineState> Dx12PipelineStateCache::Load(
        const PipelineStateBuilderC& Builder)
    {
        auto Result = Dx12PipelineStateCache::Build(Builder);

        std::scoped_lock Lock(s_PipelineStateCacheMutex);

        auto& Cache = s_PipelineStateCache[Result.Digest];
        if (!Cache)
        {
            Cache = std::make_shared<Dx12PipelineState>(Result.Desc);
        }

        return Cache;
    }

    //

    auto Dx12PipelineStateCache::Build(
        const PipelineStateBuilderG& Builder) -> GraphicsBuildResult
    {
        GraphicsBuildResult Result;

        Crypto::Sha256 Hash;

        // Root signature
        {
            auto  RootSignature     = static_cast<Dx12RootSignature*>(Builder.RootSignature.get());
            auto& RootSignatureHash = RootSignature->GetHash();
            Hash.Append(RootSignatureHash.data(), RootSignatureHash.size());

            Result.Desc.pRootSignature = RootSignature->Get();
        }

        // Shaders
        {
            for (auto [TargetShader, SrcShader] : {
                     std::pair{ &Result.Desc.VS, Builder.VertexShader.get() },
                     std::pair{ &Result.Desc.PS, Builder.PixelShader.get() },
                     std::pair{ &Result.Desc.GS, Builder.GeometryShader.get() },
                     std::pair{ &Result.Desc.HS, Builder.HullShader.get() },
                     std::pair{ &Result.Desc.DS, Builder.DomainShader.get() } })
            {
                if (SrcShader)
                {
                    auto ByteCode = SrcShader->GetByteCode();
                    *TargetShader = {
                        ByteCode.Data, ByteCode.Size
                    };
                    Hash.Append(ByteCode.Data, ByteCode.Size);
                }
                else
                {
                    *TargetShader = {};
                }

                Hash << 0xCC0139;
            }
        }

        // Blend state
        {
            auto& BlendState = Result.Desc.BlendState;
            BlendState       = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

            BlendState.AlphaToCoverageEnable  = Builder.Blend.AlphaToCoverageEnable;
            BlendState.IndependentBlendEnable = Builder.Blend.AlphaToCoverageEnable;

            for (size_t i = 0; i < std::size(Builder.Blend.RenderTargets); ++i)
            {
                auto& Src = Builder.Blend.RenderTargets[i];
                auto& Dst = BlendState.RenderTarget[i];

                Dst.BlendEnable   = Src.BlendEnable;
                Dst.LogicOpEnable = Src.LogicEnable;

                Dst.SrcBlend  = CastBlendTarget(Src.Src);
                Dst.DestBlend = CastBlendTarget(Src.Dest);
                Dst.BlendOp   = CastBlendOp(Src.OpSrc);

                Dst.SrcBlendAlpha  = CastBlendTarget(Src.Src);
                Dst.DestBlendAlpha = CastBlendTarget(Src.Dest);
                Dst.BlendOpAlpha   = CastBlendOp(Src.OpAlpha);

                Dst.LogicOp = CastLogicOp(Src.OpLogic);

                Dst.RenderTargetWriteMask = Src.WriteMask;
            }

            Hash.Append(std::bit_cast<const uint8_t*>(&BlendState), sizeof(BlendState));
        }

        // Sample mask
        {
            Result.Desc.SampleMask = Builder.SampleMask;
            Hash << Builder.SampleMask;
        }

        // Rasterizer state
        {
            auto& RasterizerState = Result.Desc.RasterizerState;
            RasterizerState       = CD3DX12_RASTERIZER_DESC(
                CastFillMode(Builder.Rasterizer.FillMode),
                CastCullMode(Builder.Rasterizer.CullMode),
                Builder.Rasterizer.FrontCounterClockwise,
                Builder.Rasterizer.DepthBias,
                Builder.Rasterizer.DepthBiasClamp,
                Builder.Rasterizer.SlopeScaledDepthBias,
                Builder.Rasterizer.DepthClipEnable,
                Builder.Rasterizer.MultisampleEnable,
                Builder.Rasterizer.AntialiasedLineEnable,
                Builder.Rasterizer.ForcedSampleCount,
                Builder.Rasterizer.ConservativeRaster ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF

            );

            Hash.Append(std::bit_cast<const uint8_t*>(&RasterizerState), sizeof(RasterizerState));
        }

        {

            auto& DepthStencilState = Result.Desc.DepthStencilState;
            DepthStencilState       = CD3DX12_DEPTH_STENCIL_DESC(
                Builder.DepthStencil.DepthEnable,
                Builder.DepthStencil.DepthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO,
                CastComparisonFunc(Builder.DepthStencil.DepthCmpFunc),
                Builder.DepthStencil.StencilEnable,
                Builder.DepthStencil.StencilReadMask,
                Builder.DepthStencil.StencilWriteMask,
                CastStencilOp(Builder.DepthStencil.StencilFrontFace.FailOp),
                CastStencilOp(Builder.DepthStencil.StencilFrontFace.DepthFailOp),
                CastStencilOp(Builder.DepthStencil.StencilFrontFace.PassOp),
                CastComparisonFunc(Builder.DepthStencil.StencilFrontFace.CmpOp),
                CastStencilOp(Builder.DepthStencil.StencilBackFace.FailOp),
                CastStencilOp(Builder.DepthStencil.StencilBackFace.DepthFailOp),
                CastStencilOp(Builder.DepthStencil.StencilBackFace.PassOp),
                CastComparisonFunc(Builder.DepthStencil.StencilBackFace.CmpOp));

            Hash.Append(std::bit_cast<const uint8_t*>(&DepthStencilState), sizeof(DepthStencilState));
        }

        // Input layout
        {
            Result.InputLayout = Builder.Input;
            auto& InputLayout  = Result.Desc.InputLayout;

            for (auto& [Name, Element] : Result.InputLayout)
            {
                auto SemanticView = Name |
                                    views::split('#') |
                                    views::take(2) |
                                    views::transform([](auto&& Range)
                                                     { return StringU8View(Range.begin(), Range.end()); }) |
                                    ranges::to<std::vector<StringU8View>>();

                auto& NameIter = SemanticView[0];

                auto& Dst        = Result.InputElements.emplace_back();
                Dst.SemanticName = NameIter.data();
                Dst.SemanticIndex =
                    SemanticView.size() > 1 ? std::strtoul(SemanticView[1].data(), nullptr, 10) : 0;
                Dst.Format               = CastFormat(Element);
                Dst.InputSlot            = 0;
                Dst.AlignedByteOffset    = D3D12_APPEND_ALIGNED_ELEMENT;
                Dst.InputSlotClass       = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                Dst.InstanceDataStepRate = 0;

                Hash << Name << Element;
            }

            if (!Result.InputElements.empty())
            {
                InputLayout.pInputElementDescs = Result.InputElements.data();
                InputLayout.NumElements        = UINT(Result.InputElements.size());
            }
        }

        // Strip cut value
        {
            switch (Builder.StripCut)
            {
            case PipelineStateBuilderG::StripCutType::None:
                Result.Desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
                break;
            case PipelineStateBuilderG::StripCutType::MaxUInt16:
                Result.Desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
                break;
            case PipelineStateBuilderG::StripCutType::MaxUInt32:
                Result.Desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF;
                break;
            }

            Hash << Result.Desc.IBStripCutValue;
        }

        // Primitive topology
        {
            switch (Builder.Topology)
            {
            case PrimitiveTopologyCategory::Undefined:
                Result.Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
                break;
            case PrimitiveTopologyCategory::Point:
                Result.Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
                break;
            case PrimitiveTopologyCategory::Line:
                Result.Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
                break;
            case PrimitiveTopologyCategory::Triangle:
                Result.Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
                break;
            case PrimitiveTopologyCategory::Patch:
                Result.Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
                break;
            }

            Hash << Result.Desc.PrimitiveTopologyType;
        }

        // Render target formats + depth stencil format
        {
            Result.Desc.NumRenderTargets = UINT(Builder.RTFormats.size());
            ranges::transform(Builder.RTFormats, Result.Desc.RTVFormats, [](auto&& Format)
                              { return CastFormat(Format); });
            Hash.Append(
                std::bit_cast<const uint8_t*>(&Result.Desc.RTVFormats[0]),
                Result.Desc.NumRenderTargets * sizeof(Result.Desc.RTVFormats[0]));

            Result.Desc.DSVFormat = CastFormat(Builder.DSFormat);
            Hash << Result.Desc.DSVFormat;
        }

        {
            Result.Desc.SampleDesc.Count   = Builder.SampleCount;
            Result.Desc.SampleDesc.Quality = Builder.SampleQuality;
            Hash.Append(
                std::bit_cast<const uint8_t*>(&Result.Desc.SampleDesc),
                sizeof(Result.Desc.SampleDesc));
        }

        Result.Desc.NodeMask  = 0;
        Result.Desc.CachedPSO = {};
        Result.Desc.Flags     = D3D12_PIPELINE_STATE_FLAG_NONE;

        Result.Digest = Hash.Digest();
        return Result;
    }

    auto Dx12PipelineStateCache::Build(
        const PipelineStateBuilderC& Builder) -> ComputeBuildResult
    {
        ComputeBuildResult Result;

        Crypto::Sha256 Hash;

        // Root signature
        {
            auto  RootSignature     = static_cast<Dx12RootSignature*>(Builder.RootSignature.get());
            auto& RootSignatureHash = RootSignature->GetHash();
            Hash.Append(RootSignatureHash.data(), RootSignatureHash.size());

            Result.Desc.pRootSignature = RootSignature->Get();
        }

        // Shader
        {
            auto ByteCode  = Builder.ComputeShader->GetByteCode();
            Result.Desc.CS = {
                ByteCode.Data, ByteCode.Size
            };
            Hash.Append(ByteCode.Data, ByteCode.Size);
        }

        Result.Desc.NodeMask  = 0;
        Result.Desc.CachedPSO = {};
        Result.Desc.Flags     = D3D12_PIPELINE_STATE_FLAG_NONE;

        Result.Digest = Hash.Digest();
        return Result;
    }
} // namespace Neon::RHI