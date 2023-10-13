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
        MRootDescriptorTableFlags Flags)
    {
        m_DescriptorRanges.emplace_back(
            RootDescriptorTableParam{
                .ShaderRegister  = BaseShaderRegister,
                .RegisterSpace   = RegisterSpace,
                .DescriptorCount = NumDescriptors,
                .Offset          = Offset,
                .Flags           = Flags,
                .Type            = DescriptorTableParam::ShaderResource });
        return *this;
    }

    RootDescriptorTable& RootDescriptorTable::AddUavRangeAt(
        uint32_t                  BaseShaderRegister,
        uint32_t                  RegisterSpace,
        uint32_t                  NumDescriptors,
        uint32_t                  Offset,
        MRootDescriptorTableFlags Flags)
    {
        m_DescriptorRanges.emplace_back(
            RootDescriptorTableParam{
                .ShaderRegister  = BaseShaderRegister,
                .RegisterSpace   = RegisterSpace,
                .DescriptorCount = NumDescriptors,
                .Offset          = Offset,
                .Flags           = Flags,
                .Type            = DescriptorTableParam::UnorderedAccess });
        return *this;
    }

    RootDescriptorTable& RootDescriptorTable::AddCbvRangeAt(
        uint32_t                  BaseShaderRegister,
        uint32_t                  RegisterSpace,
        uint32_t                  NumDescriptors,
        uint32_t                  Offset,
        MRootDescriptorTableFlags Flags)
    {
        m_DescriptorRanges.emplace_back(
            RootDescriptorTableParam{
                .ShaderRegister  = BaseShaderRegister,
                .RegisterSpace   = RegisterSpace,
                .DescriptorCount = NumDescriptors,
                .Offset          = Offset,
                .Flags           = Flags,
                .Type            = DescriptorTableParam::ConstantBuffer });
        return *this;
    }

    RootDescriptorTable& RootDescriptorTable::AddSamplerRangeAt(
        uint32_t                  BaseShaderRegister,
        uint32_t                  RegisterSpace,
        uint32_t                  NumDescriptors,
        uint32_t                  Offset,
        MRootDescriptorTableFlags Flags)
    {
        m_DescriptorRanges.emplace_back(
            RootDescriptorTableParam{
                .ShaderRegister  = BaseShaderRegister,
                .RegisterSpace   = RegisterSpace,
                .DescriptorCount = NumDescriptors,
                .Offset          = Offset,
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
        m_Parameters.emplace_back(
            RootParameter{
                std::move(Table),
                Visibility });
        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::Add32BitConstants(
        uint32_t         ShaderRegister,
        uint32_t         RegisterSpace,
        uint32_t         Num32BitValues,
        ShaderVisibility Visibility)
    {
        m_Parameters.emplace_back(
            RootParameter{
                RootParameter::Constants{
                    .ShaderRegister = ShaderRegister,
                    .RegisterSpace  = RegisterSpace,
                    .Num32BitValues = Num32BitValues },
                Visibility });
        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::AddConstantBufferView(
        uint32_t             ShaderRegister,
        uint32_t             RegisterSpace,
        ShaderVisibility     Visibility,
        MRootDescriptorFlags Flags)
    {
        m_Parameters.emplace_back(
            RootParameter{
                RootParameter::Root{
                    .ShaderRegister = ShaderRegister,
                    .RegisterSpace  = RegisterSpace,
                    .Type           = RootParameter::RootType::ConstantBuffer,
                    .Flags          = Flags },
                Visibility });
        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::AddShaderResourceView(
        uint32_t             ShaderRegister,
        uint32_t             RegisterSpace,
        ShaderVisibility     Visibility,
        MRootDescriptorFlags Flags)
    {
        m_Parameters.emplace_back(
            RootParameter{
                RootParameter::Root{
                    .ShaderRegister = ShaderRegister,
                    .RegisterSpace  = RegisterSpace,
                    .Type           = RootParameter::RootType::ShaderResource,
                    .Flags          = Flags },
                Visibility });
        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::AddUnorderedAccessView(
        uint32_t             ShaderRegister,
        uint32_t             RegisterSpace,
        ShaderVisibility     Visibility,
        MRootDescriptorFlags Flags)
    {
        m_Parameters.emplace_back(
            RootParameter{
                RootParameter::Root{
                    .ShaderRegister = ShaderRegister,
                    .RegisterSpace  = RegisterSpace,
                    .Type           = RootParameter::RootType::UnorderedAccess,
                    .Flags          = Flags },
                Visibility });
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

    //

    auto IRootSignature::Load() -> CommonRootsignatureList
    {
        CommonRootsignatureList RootSignatures;

        auto StoreRootSignature = [&RootSignatures](RSCommon::Type Type, Ptr<IRootSignature> Rs)
        {
            RootSignatures[size_t(Type)] = std::move(Rs);
        };

        //

        // RSCommon::Material
        {
            RootSignatureBuilder Builder(STR("RSCommon::Material"));
            Builder.AddShaderResourceView(0, 47); // MaterialRS::Local
            Builder.AddConstantBufferView(0, 47); // MaterialRS::SharedData
            Builder.AddShaderResourceView(0, 2);  // MaterialRS::InstanceData

            Builder.AddDescriptorTable(
                RHI::RootDescriptorTable() // MaterialRS::LightData
                    .AddSrvRange(0, 1, 1)  // _Lights
                    .AddSrvRange(1, 1, 1)  // _LightIndexList
                    .AddSrvRange(2, 1, 1), // _LightGrid
                RHI::ShaderVisibility::Pixel);
            Builder.AddConstantBufferView(0, 0, RHI::ShaderVisibility::All);

            for (auto& [IsUav, SpaceSlot] : {
                     std::pair{ true, 48 },
                     std::pair{ false, 49 },

                     std::pair{ true, 64 },
                     std::pair{ true, 65 },
                     std::pair{ true, 66 },
                     std::pair{ true, 67 },

                     std::pair{ true, 68 },
                     std::pair{ true, 69 },
                     std::pair{ true, 70 },
                     std::pair{ true, 71 },

                     std::pair{ true, 72 },
                     std::pair{ true, 73 },
                     std::pair{ true, 74 },
                     std::pair{ true, 75 },

                     std::pair{ false, 76 },

                     std::pair{ true, 77 },
                     std::pair{ true, 78 },
                     std::pair{ true, 79 },
                     std::pair{ true, 80 },

                     std::pair{ true, 81 },
                     std::pair{ true, 82 },
                     std::pair{ true, 83 },
                     std::pair{ true, 84 },

                     std::pair{ true, 85 },
                     std::pair{ true, 86 },
                     std::pair{ true, 87 },
                     std::pair{ true, 88 },

                     std::pair{ false, 89 },

                     std::pair{ true, 90 },
                     std::pair{ true, 91 },
                     std::pair{ true, 92 },
                     std::pair{ true, 93 },

                     std::pair{ true, 102 },
                     std::pair{ true, 103 },
                     std::pair{ true, 104 },
                     std::pair{ true, 105 },

                     std::pair{ true, 106 },
                     std::pair{ true, 107 },
                     std::pair{ true, 108 },
                     std::pair{ true, 109 },

                     std::pair{ true, 110 },
                     std::pair{ true, 111 },
                     std::pair{ true, 112 },
                     std::pair{ true, 113 },

                     std::pair{ false, 114 },
                     std::pair{ false, 115 } })
            {
                if (IsUav)
                {
                    Builder.AddDescriptorTable(
                        RHI::RootDescriptorTable()
                            .AddUavRange(0, SpaceSlot, std::numeric_limits<uint32_t>::max()),
                        RHI::ShaderVisibility::All);
                }
                else
                {
                    Builder.AddDescriptorTable(
                        RHI::RootDescriptorTable()
                            .AddSrvRange(0, SpaceSlot, std::numeric_limits<uint32_t>::max()),
                        RHI::ShaderVisibility::All);
                }
            }

            Builder.AddDescriptorTable(
                RHI::RootDescriptorTable()
                    .AddSamplerRange(0, 48, std::numeric_limits<uint32_t>::max()),
                RHI::ShaderVisibility::All);

            Builder.AddDescriptorTable(
                RHI::RootDescriptorTable()
                    .AddSamplerRange(0, 49, std::numeric_limits<uint32_t>::max()),
                RHI::ShaderVisibility::All);

            Builder.SetFlags(RHI::ERootSignatureBuilderFlags::AllowInputLayout);
            Builder.AddStandardSamplers();

            StoreRootSignature(RSCommon::Type::Material, Builder.Build());
        }

        //

        return RootSignatures;
    }
} // namespace Neon::RHI