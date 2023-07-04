#include <EnginePCH.hpp>
#include <Renderer/Material/VariableMap.hpp>

namespace Neon::Renderer
{
    auto MaterialVariableMap::Add(
        const StringU8& Name,
        ShaderBinding   Binding,
        MaterialVarType Type) -> View&
    {
        return m_Variables.emplace(
                              std::piecewise_construct,
                              std::forward_as_tuple(Name),
                              std::forward_as_tuple(Binding, Type))
            .first->second;
    }

    void MaterialVariableMap::Remove(
        const StringU8& Name)
    {
        m_Variables.erase(Name);
    }

    //

    void MaterialVariableMap::AddSampler(
        const StringU8&         Name,
        ShaderBinding           Binding,
        RHI::ShaderVisibility   Visibility,
        const RHI::SamplerDesc& Desc)
    {
        auto& Sampler          = m_StaticSamplers.emplace(Name, Desc).first->second;
        Sampler.ShaderRegister = Binding.Register;
        Sampler.RegisterSpace  = Binding.Space;
        Sampler.Visibility     = Visibility;
    }

    void MaterialVariableMap::AddSampler(
        const StringU8&         Name,
        ShaderBinding           Binding,
        RHI::ShaderVisibility   Visibility,
        MaterialCommon::Sampler Sampler)
    {
        RHI::SamplerDesc Desc;
        switch (Sampler)
        {
        case MaterialCommon::Sampler::PointWrap:
            Desc.Filter   = RHI::ESamplerFilter::MinMagMipPoint;
            Desc.AddressU = Desc.AddressV = Desc.AddressW = RHI::ESamplerMode::Wrap;
            break;
        case MaterialCommon::Sampler::PointClamp:
            Desc.Filter   = RHI::ESamplerFilter::MinMagMipPoint;
            Desc.AddressU = Desc.AddressV = Desc.AddressW = RHI::ESamplerMode::Clamp;
            break;
        case MaterialCommon::Sampler::LinearWrap:
            Desc.Filter   = RHI::ESamplerFilter::MinMagMipLinear;
            Desc.AddressU = Desc.AddressV = Desc.AddressW = RHI::ESamplerMode::Wrap;
            break;
        case MaterialCommon::Sampler::LinearClamp:
            Desc.Filter   = RHI::ESamplerFilter::MinMagMipLinear;
            Desc.AddressU = Desc.AddressV = Desc.AddressW = RHI::ESamplerMode::Clamp;
            break;
        case MaterialCommon::Sampler::AnisotropicWrap:
            Desc.Filter   = RHI::ESamplerFilter::Anisotropic;
            Desc.AddressU = Desc.AddressV = Desc.AddressW = RHI::ESamplerMode::Wrap;
            break;
        case MaterialCommon::Sampler::AnisotropicClamp:
            Desc.Filter   = RHI::ESamplerFilter::Anisotropic;
            Desc.AddressU = Desc.AddressV = Desc.AddressW = RHI::ESamplerMode::Clamp;
            break;
        default:
            std::unreachable();
            break;
        }

        AddSampler(Name, Binding, Visibility, Desc);
    }

    void MaterialVariableMap::RemoveSampler(
        const StringU8& Name)
    {
        m_StaticSamplers.erase(Name);
    }
} // namespace Neon::Renderer