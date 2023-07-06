#include <EnginePCH.hpp>
#include <Renderer/Material/VariableMap.hpp>

#include <Log/Logger.hpp>

namespace Neon::Renderer
{
    auto MaterialVariableMap::Add(
        const StringU8& Name,
        ShaderBinding   Binding,
        MaterialVarType Type) -> View&
    {
        auto Iter = std::ranges::find_if(
            m_Variables,
            [&Name](const auto& View)
            {
                return View.Name() == Name;
            });
        if (Iter == m_Variables.end())
        {
            return m_Variables.emplace_back(Name, Binding, Type);
        }
        else
        {
            return *Iter;
        }
    }

    void MaterialVariableMap::Remove(
        const StringU8& Name)
    {
        auto Iter = std::ranges::find_if(
            m_Variables,
            [&Name](const auto& View)
            {
                return View.Name() == Name;
            });
        if (Iter != m_Variables.end())
        {
            m_Variables.erase(Iter);
        }
    }

    //

    void MaterialVariableMap::AddStaticSampler(
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

    void MaterialVariableMap::AddStaticSampler(
        const StringU8&         Name,
        ShaderBinding           Binding,
        RHI::ShaderVisibility   Visibility,
        MaterialStates::Sampler Sampler)
    {
        RHI::SamplerDesc Desc;
        switch (Sampler)
        {
        case MaterialStates::Sampler::PointWrap:
            Desc.Filter   = RHI::ESamplerFilter::MinMagMipPoint;
            Desc.AddressU = Desc.AddressV = Desc.AddressW = RHI::ESamplerMode::Wrap;
            break;
        case MaterialStates::Sampler::PointClamp:
            Desc.Filter   = RHI::ESamplerFilter::MinMagMipPoint;
            Desc.AddressU = Desc.AddressV = Desc.AddressW = RHI::ESamplerMode::Clamp;
            break;
        case MaterialStates::Sampler::LinearWrap:
            Desc.Filter   = RHI::ESamplerFilter::MinMagMipLinear;
            Desc.AddressU = Desc.AddressV = Desc.AddressW = RHI::ESamplerMode::Wrap;
            break;
        case MaterialStates::Sampler::LinearClamp:
            Desc.Filter   = RHI::ESamplerFilter::MinMagMipLinear;
            Desc.AddressU = Desc.AddressV = Desc.AddressW = RHI::ESamplerMode::Clamp;
            break;
        case MaterialStates::Sampler::AnisotropicWrap:
            Desc.Filter   = RHI::ESamplerFilter::Anisotropic;
            Desc.AddressU = Desc.AddressV = Desc.AddressW = RHI::ESamplerMode::Wrap;
            break;
        case MaterialStates::Sampler::AnisotropicClamp:
            Desc.Filter   = RHI::ESamplerFilter::Anisotropic;
            Desc.AddressU = Desc.AddressV = Desc.AddressW = RHI::ESamplerMode::Clamp;
            break;
        default:
            std::unreachable();
            break;
        }

        AddStaticSampler(Name, Binding, Visibility, Desc);
    }

    void MaterialVariableMap::RemoveStaticSampler(
        const StringU8& Name)
    {
        m_StaticSamplers.erase(Name);
    }
} // namespace Neon::Renderer