#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Resource/Common.hpp>
#include <Private/RHI/Dx12/PipelineState.hpp>
#include <Private/RHI/Dx12/Device.hpp>
#include <Private/RHI/Dx12/RootSignature.hpp>
#include <RHI/Shader.hpp>

namespace Neon::RHI
{
    std::mutex                                   s_PipelineStateCacheMutex;
    std::map<SHA256::Bytes, Ptr<IPipelineState>> s_PipelineStateCache;

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
        CullMode Mode)
    {
        switch (Mode)
        {
        case CullMode::None:
            return D3D12_CULL_MODE_NONE;
        case CullMode::Front:
            return D3D12_CULL_MODE_FRONT;
        case CullMode::Back:
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

    /// <summary>
    /// Cast a mapped buffer type to a DXGI format.
    /// </summary>
    DXGI_FORMAT CastInputFormat(
        MBuffer::Type Type)
    {
        switch (Type)
        {
        case MBuffer::Type::Float:
            return DXGI_FORMAT_R32_FLOAT;
        case MBuffer::Type::Float2:
            return DXGI_FORMAT_R32G32_FLOAT;
        case MBuffer::Type::Float3:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case MBuffer::Type::Float4:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case MBuffer::Type::Int:
            return DXGI_FORMAT_R32_SINT;
        case MBuffer::Type::Int2:
            return DXGI_FORMAT_R32G32_SINT;
        case MBuffer::Type::Int3:
            return DXGI_FORMAT_R32G32B32_SINT;
        case MBuffer::Type::Int4:
            return DXGI_FORMAT_R32G32B32A32_SINT;
        case MBuffer::Type::UInt:
            return DXGI_FORMAT_R32_UINT;
        case MBuffer::Type::UInt2:
            return DXGI_FORMAT_R32G32_UINT;
        case MBuffer::Type::UInt3:
            return DXGI_FORMAT_R32G32B32_UINT;
        case MBuffer::Type::UInt4:
            return DXGI_FORMAT_R32G32B32A32_UINT;
        default:
            std::unreachable();
        }
    }

    //

    Ptr<IPipelineState> IPipelineState::Create(
        const PipelineStateBuilder<false>& Builder)
    {
        return Dx12PipelineStateCache::Load(Builder);
    }

    Ptr<IPipelineState> IPipelineState::Create(
        const PipelineStateBuilder<true>& Builder)
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

    //

    void Dx12PipelineStateCache::Flush()
    {
        std::scoped_lock Lock(s_PipelineStateCacheMutex);
        s_PipelineStateCache.clear();
    }

    Ptr<IPipelineState> Dx12PipelineStateCache::Load(
        const PipelineStateBuilder<false>& Builder)
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
        const PipelineStateBuilder<true>& Builder)
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
        const PipelineStateBuilder<false>& Builder) -> GraphicsBuildResult
    {
        GraphicsBuildResult Result;

        SHA256 Hash;

        // Root signature
        {
            auto  RootSignature     = static_cast<Dx12RootSignature*>(Builder.RootSignature);
            auto& RootSignatureHash = RootSignature->GetHash();
            Hash.Append(RootSignatureHash.data(), RootSignatureHash.size());

            Result.Desc.pRootSignature = RootSignature->Get();
        }

        // Shaders
        {
            for (auto [TargetShader, SrcShader] : {
                     std::pair{ &Result.Desc.VS, Builder.VertexShader },
                     std::pair{ &Result.Desc.PS, Builder.PixelShader },
                     std::pair{ &Result.Desc.GS, Builder.GeometryShader },
                     std::pair{ &Result.Desc.HS, Builder.HullShader },
                     std::pair{ &Result.Desc.DS, Builder.DomainShader } })
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

                Hash.Append(0xCC0139);
            }
        }

        // Blend state
        {
            auto& BlendState = Result.Desc.BlendState;
            BlendState       = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

            BlendState.AlphaToCoverageEnable  = Builder.BlendState.AlphaToCoverageEnable;
            BlendState.IndependentBlendEnable = Builder.BlendState.AlphaToCoverageEnable;

            for (size_t i = 0; i < std::size(Builder.BlendState.RenderTargets); ++i)
            {
                auto& Src = Builder.BlendState.RenderTargets[i];
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
            Hash.Append(Builder.SampleMask);
        }

        // Rasterizer state
        {
            auto& RasterizerState = Result.Desc.RasterizerState;
            RasterizerState       = CD3DX12_RASTERIZER_DESC(
                CastFillMode(Builder.RasterizerState.FillMode),
                CastCullMode(Builder.RasterizerState.CullMode),
                Builder.RasterizerState.FrontCounterClockwise,
                Builder.RasterizerState.DepthBias,
                Builder.RasterizerState.DepthBiasClamp,
                Builder.RasterizerState.SlopeScaledDepthBias,
                Builder.RasterizerState.DepthClipEnable,
                Builder.RasterizerState.MultisampleEnable,
                Builder.RasterizerState.AntialiasedLineEnable,
                Builder.RasterizerState.ForcedSampleCount,
                Builder.RasterizerState.ConservativeRaster ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF

            );

            Hash.Append(std::bit_cast<const uint8_t*>(&RasterizerState), sizeof(RasterizerState));
        }

        {

            auto& DepthStencilState = Result.Desc.DepthStencilState;
            DepthStencilState       = CD3DX12_DEPTH_STENCIL_DESC(
                Builder.DepthStencilState.DepthEnable,
                Builder.DepthStencilState.DepthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO,
                CastComparisonFunc(Builder.DepthStencilState.DepthCmpFunc),
                Builder.DepthStencilState.StencilEnable,
                Builder.DepthStencilState.StencilReadMask,
                Builder.DepthStencilState.StencilWriteMask,
                CastStencilOp(Builder.DepthStencilState.StencilFrontFace.FailOp),
                CastStencilOp(Builder.DepthStencilState.StencilFrontFace.DepthFailOp),
                CastStencilOp(Builder.DepthStencilState.StencilFrontFace.PassOp),
                CastComparisonFunc(Builder.DepthStencilState.StencilFrontFace.CmpOp),
                CastStencilOp(Builder.DepthStencilState.StencilBackFace.FailOp),
                CastStencilOp(Builder.DepthStencilState.StencilBackFace.DepthFailOp),
                CastStencilOp(Builder.DepthStencilState.StencilBackFace.PassOp),
                CastComparisonFunc(Builder.DepthStencilState.StencilBackFace.CmpOp));

            Hash.Append(std::bit_cast<const uint8_t*>(&DepthStencilState), sizeof(DepthStencilState));
        }

        // Input layout
        {
            auto& InputLayout = Result.Desc.InputLayout;
            auto  View        = Builder.InputLayout.GetView();
            if (auto Struct = View.AsStruct())
            {
                for (auto& [Name, Element] : Struct->NestedElements)
                {
                    auto SemanticView = Name |
                                        std::views::split('#') |
                                        std::views::take(2) |
                                        std::views::transform([](auto&& Range)
                                                              { return StringU8View(Range.begin(), Range.end()); });

                    auto NameIter  = SemanticView.begin();
                    auto IndexIter = std::next(NameIter);

                    auto& Dst        = Result.InputElements.emplace_back();
                    Dst.SemanticName = (*NameIter).data();
                    Dst.SemanticIndex =
                        IndexIter != SemanticView.end() ? std::strtoul((*IndexIter).data(), nullptr, 10) : 0;
                    Dst.Format               = CastInputFormat(Element.GetType());
                    Dst.InputSlot            = 0;
                    Dst.AlignedByteOffset    = D3D12_APPEND_ALIGNED_ELEMENT;
                    Dst.InputSlotClass       = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                    Dst.InstanceDataStepRate = 0;
                }
            }
            View.GetHashCode(Hash);

            InputLayout.pInputElementDescs = Result.InputElements.data();
            InputLayout.NumElements        = UINT(Result.InputElements.size());
        }

        // Strip cut value
        {
            switch (Builder.StripCut)
            {
            case PipelineStateBuilder<false>::StripCutType::None:
                Result.Desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
                break;
            case PipelineStateBuilder<false>::StripCutType::MaxUInt16:
                Result.Desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
                break;
            case PipelineStateBuilder<false>::StripCutType::MaxUInt32:
                Result.Desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF;
                break;
            }

            Hash.Append(uint32_t(Result.Desc.IBStripCutValue));
        }

        // Primitive topology
        {
            switch (Builder.PrimitiveTopology)
            {
            case PipelineStateBuilder<false>::Toplogy::Undefined:
                Result.Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
                break;
            case PipelineStateBuilder<false>::Toplogy::Point:
                Result.Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
                break;
            case PipelineStateBuilder<false>::Toplogy::Line:
                Result.Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
                break;
            case PipelineStateBuilder<false>::Toplogy::Triangle:
                Result.Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
                break;
            case PipelineStateBuilder<false>::Toplogy::Patch:
                Result.Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
                break;
            }
            Hash.Append(uint32_t(Result.Desc.PrimitiveTopologyType));
        }

        // Render target formats + depth stencil format
        {
            Result.Desc.NumRenderTargets = UINT(Builder.RTFormats.size());
            std::ranges::transform(Builder.RTFormats, Result.Desc.RTVFormats, [](auto&& Format)
                                   { return CastFormat(Format); });
            Hash.Append(
                std::bit_cast<const uint8_t*>(&Result.Desc.RTVFormats[0]),
                Result.Desc.NumRenderTargets * sizeof(Result.Desc.RTVFormats[0]));

            Result.Desc.DSVFormat = CastFormat(Builder.DSFormat);
            Hash.Append(uint32_t(Result.Desc.DSVFormat));
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
        const PipelineStateBuilder<true>& Builder) -> ComputeBuildResult
    {
        ComputeBuildResult Result;

        SHA256 Hash;

        // Root signature
        {
            auto  RootSignature     = static_cast<Dx12RootSignature*>(Builder.RootSignature);
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