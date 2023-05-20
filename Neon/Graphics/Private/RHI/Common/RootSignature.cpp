#include <GraphicsPCH.hpp>
#include <RHI/RootSignature.hpp>

namespace Neon::RHI
{
    RootDescriptorTable::RootDescriptorTable(
        size_t Reserve)
    {
        m_DescriptorRanges.reserve(Reserve);
    }

    RootDescriptorTable& RootDescriptorTable::AddSrvRange(
        uint32_t                  BaseShaderRegister,
        uint32_t                  RegisterSpace,
        uint32_t                  NumDescriptors,
        MRootDescriptorTableFlags Flags)
    {
        m_DescriptorRanges.emplace_back(RootDescriptorTableParam{
            .ShaderRegister  = BaseShaderRegister,
            .RegisterSpace   = RegisterSpace,
            .DescriptorCount = NumDescriptors,
            .Flags           = Flags,
            .Type            = DescriptorTableParam::ShaderResource });
        return *this;
    }

    RootDescriptorTable& RootDescriptorTable::AddUavRange(
        uint32_t                  BaseShaderRegister,
        uint32_t                  RegisterSpace,
        uint32_t                  NumDescriptors,
        MRootDescriptorTableFlags Flags)
    {
        m_DescriptorRanges.emplace_back(RootDescriptorTableParam{
            .ShaderRegister  = BaseShaderRegister,
            .RegisterSpace   = RegisterSpace,
            .DescriptorCount = NumDescriptors,
            .Flags           = Flags,
            .Type            = DescriptorTableParam::UnorderedAccess });
        return *this;
    }

    RootDescriptorTable& RootDescriptorTable::AddCbvRange(
        uint32_t                  BaseShaderRegister,
        uint32_t                  RegisterSpace,
        uint32_t                  NumDescriptors,
        MRootDescriptorTableFlags Flags)
    {
        m_DescriptorRanges.emplace_back(RootDescriptorTableParam{
            .ShaderRegister  = BaseShaderRegister,
            .RegisterSpace   = RegisterSpace,
            .DescriptorCount = NumDescriptors,
            .Flags           = Flags,
            .Type            = DescriptorTableParam::ConstantBuffer });
        return *this;
    }

    RootDescriptorTable& RootDescriptorTable::AddSamplerRange(
        uint32_t                  BaseShaderRegister,
        uint32_t                  RegisterSpace,
        uint32_t                  NumDescriptors,
        MRootDescriptorTableFlags Flags)
    {
        m_DescriptorRanges.emplace_back(RootDescriptorTableParam{
            .ShaderRegister  = BaseShaderRegister,
            .RegisterSpace   = RegisterSpace,
            .DescriptorCount = NumDescriptors,
            .Flags           = Flags,
            .Type            = DescriptorTableParam::Sampler });
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
                .Num32BitValues = Num32BitValues,
                .RegisterSpace  = RegisterSpace },
            Visibility);
        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::AddConstantBufferView(
        uint32_t         ShaderRegister,
        uint32_t         RegisterSpace,
        uint32_t         Num32BitValues,
        ShaderVisibility Visibility)
    {
        m_Parameters.emplace_back(
            RootParameter::Descriptor{
                .ShaderRegister = ShaderRegister,
                .RegisterSpace  = RegisterSpace,
                .Type           = RootParameter::DescriptorType::ConstantBuffer },
            Visibility);
        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::AddShaderResourceView(
        uint32_t         ShaderRegister,
        uint32_t         RegisterSpace,
        ShaderVisibility Visibility)
    {
        m_Parameters.emplace_back(
            RootParameter::Descriptor{
                .ShaderRegister = ShaderRegister,
                .RegisterSpace  = RegisterSpace,
                .Type           = RootParameter::DescriptorType::ShaderResource },
            Visibility);
        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::AddUnorderedAccessView(
        uint32_t         ShaderRegister,
        uint32_t         RegisterSpace,
        ShaderVisibility Visibility)
    {
        m_Parameters.emplace_back(
            RootParameter::Descriptor{
                .ShaderRegister = ShaderRegister,
                .RegisterSpace  = RegisterSpace,
                .Type           = RootParameter::DescriptorType::UnorderedAccess },
            Visibility);
        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::AddUnorderedAccessViewWithCounter(
        uint32_t                  ShaderRegister,
        uint32_t                  RegisterSpace,
        MRootDescriptorTableFlags Flags,
        ShaderVisibility          Visibility)
    {
        RootDescriptorTable Table(1);
        Table.AddUavRange(ShaderRegister, RegisterSpace, 1, Flags);
        return AddDescriptorTable(Table, Visibility);
    }

    RootSignatureBuilder& RootSignatureBuilder::AddSampler(
        const StaticSamplerDesc& Desc)
    {
        m_StaticSamplers.emplace_back(Desc);
        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::SetFlags(
        ERootSignatureBuilderFlags Flag,
        bool                       Value)
    {
        m_Flags.Set(Flag, Value);
        return *this;
    }
} // namespace Neon::RHI