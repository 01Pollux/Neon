#include <GraphicsPCH.hpp>
#include <RHI/RootSignature.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
    RootDescriptorTable& RootDescriptorTable::AddSrvRangeAt(
        uint32_t                  BaseShaderRegister,
        uint32_t                  RegisterSpace,
        uint32_t                  NumDescriptors,
        uint32_t                  Offset,
        bool                      Instanced,
        MRootDescriptorTableFlags Flags)
    {
        m_DescriptorRanges.emplace_back(
            RootDescriptorTableParam{
                .ShaderRegister  = BaseShaderRegister,
                .RegisterSpace   = RegisterSpace,
                .DescriptorCount = NumDescriptors,
                .Offset          = Offset,
                .Flags           = Flags,
                .Type            = DescriptorTableParam::ShaderResource,
                .Instanced       = Instanced });
        return *this;
    }

    RootDescriptorTable& RootDescriptorTable::AddUavRangeAt(
        uint32_t                  BaseShaderRegister,
        uint32_t                  RegisterSpace,
        uint32_t                  NumDescriptors,
        uint32_t                  Offset,
        bool                      Instanced,
        MRootDescriptorTableFlags Flags)
    {
        m_DescriptorRanges.emplace_back(
            RootDescriptorTableParam{
                .ShaderRegister  = BaseShaderRegister,
                .RegisterSpace   = RegisterSpace,
                .DescriptorCount = NumDescriptors,
                .Offset          = Offset,
                .Flags           = Flags,
                .Type            = DescriptorTableParam::UnorderedAccess,
                .Instanced       = Instanced });
        return *this;
    }

    RootDescriptorTable& RootDescriptorTable::AddCbvRangeAt(
        uint32_t                  BaseShaderRegister,
        uint32_t                  RegisterSpace,
        uint32_t                  NumDescriptors,
        uint32_t                  Offset,
        bool                      Instanced,
        MRootDescriptorTableFlags Flags)
    {
        m_DescriptorRanges.emplace_back(
            RootDescriptorTableParam{
                .ShaderRegister  = BaseShaderRegister,
                .RegisterSpace   = RegisterSpace,
                .DescriptorCount = NumDescriptors,
                .Offset          = Offset,
                .Flags           = Flags,
                .Type            = DescriptorTableParam::ConstantBuffer,
                .Instanced       = Instanced });
        return *this;
    }

    RootDescriptorTable& RootDescriptorTable::AddSamplerRangeAt(
        uint32_t                  BaseShaderRegister,
        uint32_t                  RegisterSpace,
        uint32_t                  NumDescriptors,
        uint32_t                  Offset,
        bool                      Instanced,
        MRootDescriptorTableFlags Flags)
    {
        m_DescriptorRanges.emplace_back(
            RootDescriptorTableParam{
                .ShaderRegister  = BaseShaderRegister,
                .RegisterSpace   = RegisterSpace,
                .DescriptorCount = NumDescriptors,
                .Offset          = Offset,
                .Flags           = Flags,
                .Type            = DescriptorTableParam::Sampler,
                .Instanced       = Instanced });
        return *this;
    }

    //

    RootParameter::RootParameter(
        Variant          Param,
        ShaderVisibility Visibility) :
        m_Parameter(std::move(Param)),
        m_Visibility(Visibility)
    {
    }

    //

    RootSignatureBuilder& RootSignatureBuilder::AddDescriptorTable(
        RootDescriptorTable Table,
        ShaderVisibility    Visibility)
    {
        m_Parameters.emplace_back(std::move(Table), Visibility);
        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::Add32BitConstants(
        uint32_t         ShaderRegister,
        uint32_t         RegisterSpace,
        uint32_t         Num32BitValues,
        ShaderVisibility Visibility)
    {
        m_Parameters.emplace_back(
            RootParameter::Constants{
                .ShaderRegister = ShaderRegister,
                .RegisterSpace  = RegisterSpace,
                .Num32BitValues = Num32BitValues },
            Visibility);
        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::AddConstantBufferView(
        uint32_t             ShaderRegister,
        uint32_t             RegisterSpace,
        ShaderVisibility     Visibility,
        MRootDescriptorFlags Flags)
    {
        m_Parameters.emplace_back(
            RootParameter::Root{
                .ShaderRegister = ShaderRegister,
                .RegisterSpace  = RegisterSpace,
                .Type           = RootParameter::RootType::ConstantBuffer,
                .Flags          = Flags },
            Visibility);
        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::AddShaderResourceView(
        uint32_t             ShaderRegister,
        uint32_t             RegisterSpace,
        ShaderVisibility     Visibility,
        MRootDescriptorFlags Flags)
    {
        m_Parameters.emplace_back(
            RootParameter::Root{
                .ShaderRegister = ShaderRegister,
                .RegisterSpace  = RegisterSpace,
                .Type           = RootParameter::RootType::ShaderResource,
                .Flags          = Flags },
            Visibility);
        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::AddUnorderedAccessView(
        uint32_t             ShaderRegister,
        uint32_t             RegisterSpace,
        ShaderVisibility     Visibility,
        MRootDescriptorFlags Flags)
    {
        m_Parameters.emplace_back(
            RootParameter::Root{
                .ShaderRegister = ShaderRegister,
                .RegisterSpace  = RegisterSpace,
                .Type           = RootParameter::RootType::UnorderedAccess,
                .Flags          = Flags },
            Visibility);
        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::AddSampler(
        const StaticSamplerDesc& Desc)
    {
        m_StaticSamplers.emplace_back(Desc);
        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::AddStandardSamplers(
        uint32_t         RegisterSpace,
        ShaderVisibility Visibility)
    {
        StaticSamplerDesc Desc;
        Desc.RegisterSpace = RegisterSpace;
        Desc.Visibility    = Visibility;

        enum class StandardSamplerType : uint8_t
        {
            PointWrap,
            PointClamp,

            LinearWrap,
            LinearClamp,
            LinearBorder,

            AnisotropicWrap,
            AnisotropicClamp,

            _Count
        };

        for (uint32_t i = 0; i < uint32_t(StandardSamplerType::_Count); i++)
        {
            Desc.ShaderRegister = i;

            switch (StandardSamplerType(i))
            {
            case StandardSamplerType::PointWrap:
                Desc.Filter   = RHI::ESamplerFilter::MinMagMipPoint;
                Desc.AddressU = Desc.AddressV = Desc.AddressW = RHI::ESamplerMode::Wrap;
                break;
            case StandardSamplerType::PointClamp:
                Desc.Filter   = RHI::ESamplerFilter::MinMagMipPoint;
                Desc.AddressU = Desc.AddressV = Desc.AddressW = RHI::ESamplerMode::Clamp;
                break;
            case StandardSamplerType::LinearWrap:
                Desc.Filter   = RHI::ESamplerFilter::MinMagMipLinear;
                Desc.AddressU = Desc.AddressV = Desc.AddressW = RHI::ESamplerMode::Wrap;
                break;
            case StandardSamplerType::LinearClamp:
                Desc.Filter   = RHI::ESamplerFilter::MinMagMipLinear;
                Desc.AddressU = Desc.AddressV = Desc.AddressW = RHI::ESamplerMode::Clamp;
                break;
            case StandardSamplerType::LinearBorder:
                Desc.Filter   = RHI::ESamplerFilter::MinMagMipLinear;
                Desc.AddressU = Desc.AddressV = Desc.AddressW = RHI::ESamplerMode::Border;
                break;
            case StandardSamplerType::AnisotropicWrap:
                Desc.Filter   = RHI::ESamplerFilter::Anisotropic;
                Desc.AddressU = Desc.AddressV = Desc.AddressW = RHI::ESamplerMode::Wrap;
                break;
            case StandardSamplerType::AnisotropicClamp:
                Desc.Filter   = RHI::ESamplerFilter::Anisotropic;
                Desc.AddressU = Desc.AddressV = Desc.AddressW = RHI::ESamplerMode::Clamp;
                break;
            }

            AddSampler(Desc);
        }

        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::SetFlags(
        ERootSignatureBuilderFlags Flag,
        bool                       Value)
    {
        m_Flags.Set(Flag, Value);
        return *this;
    }

    Ptr<IRootSignature> RootSignatureBuilder::Build() const
    {
        return IRootSignature::Create(*this);
    }
} // namespace Neon::RHI