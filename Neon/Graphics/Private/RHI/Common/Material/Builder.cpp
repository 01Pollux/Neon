#include <GraphicsPCH.hpp>
#include <RHI/Material/Builder.hpp>
#include <RHI/Material/Material.hpp>

namespace Neon::RHI
{
    Ptr<IMaterial> RenderMaterialBuilder::Build() const
    {
        return IMaterial::Create(*this);
    }

    Ptr<IMaterial> ComputeMaterialBuilder::Build() const
    {
        return IMaterial::Create(*this);
    }

    //

    RenderMaterialBuilder& RenderMaterialBuilder::NoVertexInput(
        bool NoInput)
    {
        m_NoVertexInput = NoInput;
        return *this;
    }

    bool RenderMaterialBuilder::NoVertexInput() const
    {
        return m_NoVertexInput;
    }

    //

#define MATERIAL_SHADER_FUNC(MaterialClass, ShaderStage, Index)  \
    MaterialClass& MaterialClass::ShaderStage(                   \
        const Ptr<IShader>& Shader)                              \
    {                                                            \
        m_ShaderModules[Index] = Shader;                         \
        return *this;                                            \
    }                                                            \
                                                                 \
    auto MaterialClass::ShaderStage() const->const Ptr<IShader>& \
    {                                                            \
        return m_ShaderModules[Index];                           \
    }

    //

    MATERIAL_SHADER_FUNC(RenderMaterialBuilder, VertexShader, 0);
    MATERIAL_SHADER_FUNC(RenderMaterialBuilder, HullShader, 1);
    MATERIAL_SHADER_FUNC(RenderMaterialBuilder, DomainShader, 2);
    MATERIAL_SHADER_FUNC(RenderMaterialBuilder, GeometryShader, 3);
    MATERIAL_SHADER_FUNC(RenderMaterialBuilder, PixelShader, 4);

    MATERIAL_SHADER_FUNC(ComputeMaterialBuilder, ComputeShader, 0);

#undef MATERIAL_SHADER_FUNC

    //

#define MATERIAL_GET_SET_FUNC(MaterialClass, FuncName, MemberName)                   \
    MaterialClass& MaterialClass::FuncName(                                          \
        decltype(MaterialClass::MemberName) Value)                                   \
    {                                                                                \
        MemberName = std::move(Value);                                               \
        return *this;                                                                \
    }                                                                                \
                                                                                     \
    auto MaterialClass::FuncName() const->const decltype(MaterialClass::MemberName)& \
    {                                                                                \
        return MemberName;                                                           \
    }

    //

    MATERIAL_GET_SET_FUNC(RenderMaterialBuilder, Blend, m_BlendState);
    MATERIAL_GET_SET_FUNC(RenderMaterialBuilder, Rasterizer, m_Rasterizer);
    MATERIAL_GET_SET_FUNC(RenderMaterialBuilder, DepthStencil, m_DepthStencil);
    MATERIAL_GET_SET_FUNC(RenderMaterialBuilder, InputLayout, m_InputLayout);

    //

    RenderMaterialBuilder& RenderMaterialBuilder::Blend(
        size_t                Index,
        MaterialStates::Blend State)
    {
        auto& Desc = m_BlendState.RenderTargets[Index];
        switch (State)
        {
        case MaterialStates::Blend::Opaque:
            break;
        case MaterialStates::Blend::AlphaBlend:
            Desc.BlendEnable = true;
            Desc.Src         = BlendTarget::SrcAlpha;
            Desc.Dest        = BlendTarget::InvSrcAlpha;
            Desc.OpSrc       = BlendOp::Add;
            Desc.SrcAlpha    = BlendTarget::One;
            Desc.DestAlpha   = BlendTarget::InvSrcAlpha;
            Desc.OpAlpha     = BlendOp::Add;
            break;
        case MaterialStates::Blend::Additive:
            Desc.BlendEnable = true;
            Desc.Src         = BlendTarget::SrcAlpha;
            Desc.Dest        = BlendTarget::One;
            Desc.OpSrc       = BlendOp::Add;
            Desc.SrcAlpha    = BlendTarget::SrcAlpha;
            Desc.DestAlpha   = BlendTarget::One;
            Desc.OpAlpha     = BlendOp::Add;
            break;
        case MaterialStates::Blend::NonPremultiplied:
            Desc.BlendEnable = true;
            Desc.Src         = BlendTarget::SrcAlpha;
            Desc.Dest        = BlendTarget::InvSrcAlpha;
            Desc.OpSrc       = BlendOp::Add;
            Desc.SrcAlpha    = BlendTarget::SrcAlpha;
            Desc.DestAlpha   = BlendTarget::InvSrcAlpha;
            Desc.OpAlpha     = BlendOp::Add;
            break;
        default:
            std::unreachable();
        }
        return *this;
    }

    //

    RenderMaterialBuilder& RenderMaterialBuilder::Sample(
        uint32_t Mask,
        uint32_t Count,
        uint32_t Quality)
    {
        m_SampleMask    = Mask;
        m_SampleCount   = Count;
        m_SampleQuality = Quality;
        return *this;
    }

    uint32_t RenderMaterialBuilder::SampleMask() const
    {
        return m_SampleMask;
    }

    uint32_t RenderMaterialBuilder::SampleCount() const
    {
        return m_SampleCount;
    }

    uint32_t RenderMaterialBuilder::SampleQuality() const
    {
        return m_SampleQuality;
    }

    //

    RenderMaterialBuilder& RenderMaterialBuilder::Rasterizer(
        MaterialStates::Rasterizer State)
    {
        PipelineStateBuilderG::RasterizerState Desc;
        switch (State)
        {
        case MaterialStates::Rasterizer::CullNone:
            Desc.CullMode = ECullMode::None;
            break;
        case MaterialStates::Rasterizer::CullClockwise:
            Desc.CullMode = ECullMode::Front;
            break;
        case MaterialStates::Rasterizer::CullCounterClockwise:
            Desc.CullMode = ECullMode::Back;
            break;
        case MaterialStates::Rasterizer::Wireframe:
            Desc.FillMode = FillMode::Wireframe;
            Desc.CullMode = ECullMode::None;
            break;
        default:
            std::unreachable();
        }
        return Rasterizer(std::move(Desc));
    }

    //

    RenderMaterialBuilder& RenderMaterialBuilder::DepthStencil(
        MaterialStates::DepthStencil State)
    {
        PipelineStateBuilderG::DepthStencilState Desc;
        switch (State)
        {
        case MaterialStates::DepthStencil::None:
            Desc.DepthEnable      = false;
            Desc.DepthWriteEnable = false;
            break;
        case MaterialStates::DepthStencil::Default:
            break;
        case MaterialStates::DepthStencil::Read:
            Desc.DepthWriteEnable = false;
            break;
        case MaterialStates::DepthStencil::ReverseZ:
            Desc.DepthCmpFunc = ECompareFunc::Greater;
            break;
        case MaterialStates::DepthStencil::ReadReverseZ:
            Desc.DepthWriteEnable = false;
            Desc.DepthCmpFunc     = ECompareFunc::Greater;
            break;
        default:
            std::unreachable();
        }
        return DepthStencil(std::move(Desc));
    }

    //

    RenderMaterialBuilder& RenderMaterialBuilder::StripCut(
        PipelineStateBuilderG::StripCutType Type)
    {
        m_StripCut = Type;
        return *this;
    }

    PipelineStateBuilderG::StripCutType RenderMaterialBuilder::StripCut() const
    {
        return m_StripCut;
    }

    //

    RenderMaterialBuilder& RenderMaterialBuilder::RenderTarget(
        uint32_t        Index,
        EResourceFormat Format)
    {
        m_RenderTargetFormats[Index] = Format;
        return *this;
    }

    //

    RenderMaterialBuilder& RenderMaterialBuilder::Topology(
        PrimitiveTopologyCategory Type)
    {
        m_Topology = Type;
        return *this;
    }

    PrimitiveTopologyCategory RenderMaterialBuilder::Topology() const
    {
        return m_Topology;
    }

    //

    RenderMaterialBuilder& RenderMaterialBuilder::DepthStencilFormat(
        EResourceFormat Format)
    {
        m_DepthStencilFormat = Format;
        return *this;
    }

    EResourceFormat RenderMaterialBuilder::DepthStencilFormat() const
    {
        return m_DepthStencilFormat;
    }

    //

    SamplerDesc GetSamplerDesc(
        MaterialStates::Sampler Type)
    {
        SamplerDesc Desc;
        switch (Type)
        {
        case MaterialStates::Sampler::PointWrap:
            Desc.Filter   = ESamplerFilter::MinMagMipPoint;
            Desc.AddressU = Desc.AddressV = Desc.AddressW = ESamplerMode::Wrap;
            break;
        case MaterialStates::Sampler::PointClamp:
            Desc.Filter   = ESamplerFilter::MinMagMipPoint;
            Desc.AddressU = Desc.AddressV = Desc.AddressW = ESamplerMode::Clamp;
            break;
        case MaterialStates::Sampler::LinearWrap:
            Desc.Filter   = ESamplerFilter::MinMagMipLinear;
            Desc.AddressU = Desc.AddressV = Desc.AddressW = ESamplerMode::Wrap;
            break;
        case MaterialStates::Sampler::LinearClamp:
            Desc.Filter   = ESamplerFilter::MinMagMipLinear;
            Desc.AddressU = Desc.AddressV = Desc.AddressW = ESamplerMode::Clamp;
            break;
        case MaterialStates::Sampler::AnisotropicWrap:
            Desc.Filter   = ESamplerFilter::Anisotropic;
            Desc.AddressU = Desc.AddressV = Desc.AddressW = ESamplerMode::Wrap;
            break;
        case MaterialStates::Sampler::AnisotropicClamp:
            Desc.Filter   = ESamplerFilter::Anisotropic;
            Desc.AddressU = Desc.AddressV = Desc.AddressW = ESamplerMode::Clamp;
            break;
        default:
            std::unreachable();
            break;
        }
        return Desc;
    }

    StaticSamplerDesc GetStaticSamplerDesc(
        MaterialStates::Sampler Type,
        uint16_t                Register,
        uint16_t                Space,
        ShaderVisibility        Visibility)
    {
        auto Desc           = StaticSamplerDesc(GetSamplerDesc(Type));
        Desc.ShaderRegister = Register;
        Desc.RegisterSpace  = Space;
        Desc.Visibility     = Visibility;
        return Desc;
    }
} // namespace Neon::RHI