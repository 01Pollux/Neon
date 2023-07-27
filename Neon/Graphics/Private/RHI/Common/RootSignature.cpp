#include <GraphicsPCH.hpp>
#include <RHI/RootSignature.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
    RootDescriptorTable& RootDescriptorTable::AddSrvRange(
        StringU8                  Name,
        uint32_t                  BaseShaderRegister,
        uint32_t                  RegisterSpace,
        uint32_t                  NumDescriptors,
        bool                      Instanced,
        MRootDescriptorTableFlags Flags)
    {
        m_DescriptorRanges.emplace_back(
            std::move(Name),
            RootDescriptorTableParam{
                .ShaderRegister  = BaseShaderRegister,
                .RegisterSpace   = RegisterSpace,
                .DescriptorCount = NumDescriptors,
                .Flags           = Flags,
                .Type            = DescriptorTableParam::ShaderResource,
                .Instanced       = Instanced });
        return *this;
    }

    RootDescriptorTable& RootDescriptorTable::AddUavRange(
        StringU8                  Name,
        uint32_t                  BaseShaderRegister,
        uint32_t                  RegisterSpace,
        uint32_t                  NumDescriptors,
        bool                      Instanced,
        MRootDescriptorTableFlags Flags)
    {
        m_DescriptorRanges.emplace_back(
            std::move(Name),
            RootDescriptorTableParam{
                .ShaderRegister  = BaseShaderRegister,
                .RegisterSpace   = RegisterSpace,
                .DescriptorCount = NumDescriptors,
                .Flags           = Flags,
                .Type            = DescriptorTableParam::UnorderedAccess,
                .Instanced       = Instanced });
        return *this;
    }

    RootDescriptorTable& RootDescriptorTable::AddCbvRange(
        StringU8                  Name,
        uint32_t                  BaseShaderRegister,
        uint32_t                  RegisterSpace,
        uint32_t                  NumDescriptors,
        bool                      Instanced,
        MRootDescriptorTableFlags Flags)
    {
        m_DescriptorRanges.emplace_back(
            std::move(Name),
            RootDescriptorTableParam{
                .ShaderRegister  = BaseShaderRegister,
                .RegisterSpace   = RegisterSpace,
                .DescriptorCount = NumDescriptors,
                .Flags           = Flags,
                .Type            = DescriptorTableParam::ConstantBuffer,
                .Instanced       = Instanced });
        return *this;
    }

    RootDescriptorTable& RootDescriptorTable::AddSamplerRange(
        StringU8                  Name,
        uint32_t                  BaseShaderRegister,
        uint32_t                  RegisterSpace,
        uint32_t                  NumDescriptors,
        bool                      Instanced,
        MRootDescriptorTableFlags Flags)
    {
        m_DescriptorRanges.emplace_back(
            std::move(Name),
            RootDescriptorTableParam{
                .ShaderRegister  = BaseShaderRegister,
                .RegisterSpace   = RegisterSpace,
                .DescriptorCount = NumDescriptors,
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
        StringU8         Name,
        uint32_t         ShaderRegister,
        uint32_t         RegisterSpace,
        uint32_t         Num32BitValues,
        ShaderVisibility Visibility)
    {
        m_Parameters.emplace_back(
            RootParameter::Constants{
                .Name           = std::move(Name),
                .ShaderRegister = ShaderRegister,
                .RegisterSpace  = RegisterSpace,
                .Num32BitValues = Num32BitValues },
            Visibility);
        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::AddConstantBufferView(
        StringU8             Name,
        uint32_t             ShaderRegister,
        uint32_t             RegisterSpace,
        ShaderVisibility     Visibility,
        MRootDescriptorFlags Flags)
    {
        m_Parameters.emplace_back(
            RootParameter::Root{
                .Name           = std::move(Name),
                .ShaderRegister = ShaderRegister,
                .RegisterSpace  = RegisterSpace,
                .Type           = RootParameter::RootType::ConstantBuffer,
                .Flags          = Flags },
            Visibility);
        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::AddShaderResourceView(
        StringU8             Name,
        uint32_t             ShaderRegister,
        uint32_t             RegisterSpace,
        ShaderVisibility     Visibility,
        MRootDescriptorFlags Flags)
    {
        m_Parameters.emplace_back(
            RootParameter::Root{
                .Name           = std::move(Name),
                .ShaderRegister = ShaderRegister,
                .RegisterSpace  = RegisterSpace,
                .Type           = RootParameter::RootType::ShaderResource,
                .Flags          = Flags },
            Visibility);
        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::AddUnorderedAccessView(
        StringU8             Name,
        uint32_t             ShaderRegister,
        uint32_t             RegisterSpace,
        ShaderVisibility     Visibility,
        MRootDescriptorFlags Flags)
    {
        m_Parameters.emplace_back(
            RootParameter::Root{
                .Name           = std::move(Name),
                .ShaderRegister = ShaderRegister,
                .RegisterSpace  = RegisterSpace,
                .Type           = RootParameter::RootType::UnorderedAccess,
                .Flags          = Flags },
            Visibility);
        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::AddSampler(
        StringU8                 Name,
        const StaticSamplerDesc& Desc)
    {
        m_StaticSamplers.emplace_back(std::move(Name), Desc);
        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::SetFlags(
        ERootSignatureBuilderFlags Flag,
        bool                       Value)
    {
        m_Flags.Set(Flag, Value);
        return *this;
    }

    void RootSignatureBuilder::RemoveParameter(
        const StringU8& Name)
    {
        bool Found      = false;
        bool MustRemove = false;

        for (auto It = m_Parameters.begin(); It != m_Parameters.end(); It++)
        {
            boost::apply_visitor(
                VariantVisitor{
                    [&Name, &Found, &MustRemove](RootParameter::DescriptorTable& Table)
                    {
                        auto& Ranges = Table.GetRanges();
                        for (auto It = Ranges.begin(); It != Ranges.end(); It++)
                        {
                            if (It->first == Name)
                            {
                                Ranges.erase(It);
                                Found      = true;
                                MustRemove = Ranges.empty();
                                break;
                            }
                        }
                    },
                    [&Name, &Found](RootParameter::Constants& Constant)
                    {
                        Found = Constant.Name == Name;
                    },
                    [&Name, &Found](RootParameter::Root& Root)
                    {
                        Found = Root.Name == Name;
                    } },
                It->GetParameter());

            if (Found)
            {
                if (MustRemove)
                {
                    m_Parameters.erase(It);
                }
                return;
            }
        }

        NEON_WARNING("Graphics", "Root signature parameter '{}' not found", Name);
    }

    void RootSignatureBuilder::RemoveStaticSampler(
        const StringU8& Name)
    {
        for (auto It = m_StaticSamplers.begin(); It != m_StaticSamplers.end(); It++)
        {
            if (It->first == Name)
            {
                m_StaticSamplers.erase(It);
                return;
            }
        }

        NEON_WARNING("Graphics", "Root signature static sampler '{}' not found", Name);
    }
} // namespace Neon::RHI