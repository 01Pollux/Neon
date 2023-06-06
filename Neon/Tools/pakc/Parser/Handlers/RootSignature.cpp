#include <PakCPCH.hpp>
#include <Parser/Handlers/RootSignature.hpp>
#include <Parser/RHI.hpp>
#include <fstream>

#include <RHI/RootSignature.hpp>
#include <Resource/Types/RootSignature.hpp>
#include <RHI/RootSignature.hpp>

#include <Log/Logger.hpp>

namespace PakC::Handler
{
    using namespace Neon;

    RHI::ShaderVisibility ConvertShaderVisibility(
        const boost::json::string& Visibility)
    {
        switch (StringUtils::Hash(StringU8(Visibility)))
        {
        case StringUtils::Hash("All"):
        {
            return RHI::ShaderVisibility::All;
        }
        case StringUtils::Hash("Vertex"):
        {
            return RHI::ShaderVisibility::Vertex;
        }
        case StringUtils::Hash("Hull"):
        {
            return RHI::ShaderVisibility::Hull;
        }
        case StringUtils::Hash("Domain"):
        {
            return RHI::ShaderVisibility::Domain;
        }
        case StringUtils::Hash("Geometry"):
        {
            return RHI::ShaderVisibility::Geometry;
        }
        case StringUtils::Hash("Pixel"):
        {
            return RHI::ShaderVisibility::Pixel;
        }
        default:
        {
            throw std::runtime_error("Invalid shader visibility");
        }
        }
    }

    //

    static RHI::MRootDescriptorTableFlags GetRangeFlags(
        const boost::json::array& Flags)
    {
        RHI::MRootDescriptorTableFlags Res;
        for (auto Flag : Flags |
                             std::views::transform([](auto&& Flag) -> decltype(auto)
                                                   { return StringU8(Flag.as_string()); }))
        {
            switch (StringUtils::Hash(Flag))
            {
            case StringUtils::Hash("Descriptor Volatile"):
            {
                Res.Set(RHI::ERootDescriptorTableFlags::Descriptor_Volatile);
                break;
            }
            case StringUtils::Hash("Descriptor Static Bounds Check"):
            {
                Res.Set(RHI::ERootDescriptorTableFlags::Descriptor_Static_Bounds_Check);
            }
            case StringUtils::Hash("Data Volatile"):
            {
                Res.Set(RHI::ERootDescriptorTableFlags::Data_Volatile);
                break;
            }
            case StringUtils::Hash("Data Static While Execute"):
            {
                Res.Set(RHI::ERootDescriptorTableFlags::Data_Static_While_Execute);
                break;
            }
            case StringUtils::Hash("Data Static"):
            {
                Res.Set(RHI::ERootDescriptorTableFlags::Data_Static);
                break;
            }
            }
        }
        return Res;
    }

    //

    static RHI::MRootDescriptorFlags GetRootFlags(
        const boost::json::array& Flags)
    {
        RHI::MRootDescriptorFlags Res;
        for (auto Flag : Flags |
                             std::views::transform([](auto&& Flag) -> decltype(auto)
                                                   { return StringU8(Flag.as_string()); }))
        {
            switch (StringUtils::Hash(Flag))
            {
            case StringUtils::Hash("Descriptor Volatile"):
            {
                Res.Set(RHI::ERootDescriptorFlags::Data_Volatile);
                break;
            }
            case StringUtils::Hash("Data Static While Execute"):
            {
                Res.Set(RHI::ERootDescriptorFlags::Data_Static_While_Execute);
            }
            case StringUtils::Hash("Data Volatile"):
            {
                Res.Set(RHI::ERootDescriptorFlags::Data_Volatile);
                break;
            }
            case StringUtils::Hash("Data Static"):
            {
                Res.Set(RHI::ERootDescriptorFlags::Data_Static);
                break;
            }
            }
        }
        return Res;
    }

    //

    AssetResourcePtr LoadRootSignatureResource(
        const boost::json::object& Object)
    {
        RHI::RootSignatureBuilder Builder;

        if (auto Params = Object.find("Root Parameters"); Params != Object.end())
        {
            for (auto& Parameter : Params->value().as_array() |
                                       std::views::transform([](auto&& Parameter) -> decltype(auto)
                                                             { return Parameter.as_object(); }))
            {
                auto ParamType = StringU8(Parameter.at("Type").as_string());
                switch (StringUtils::Hash(ParamType))
                {
                case StringUtils::Hash("DescriptorTable"):
                {
                    RHI::RootDescriptorTable Table;

                    for (auto& Range : Parameter.at("Ranges").as_array() |
                                           std::views::transform([](auto&& Range) -> decltype(auto)
                                                                 { return Range.as_object(); }))
                    {
                        auto RangeType      = StringU8(Range.at("Range Type").as_string());
                        auto RangeCount     = uint32_t(Range.at("Count").as_int64());
                        auto ShaderRegister = uint32_t(Range.at("Shader Register").as_int64());
                        auto RegisterSpace  = uint32_t(Range.at("Register Space").as_int64());
                        auto Flags          = GetRangeFlags(Range.at("Flags").as_array());

                        switch (StringUtils::Hash(RangeType))
                        {
                        case StringUtils::Hash("ConstantBuffer"):
                        {
                            Table.AddCbvRange(
                                ShaderRegister,
                                RegisterSpace,
                                RangeCount,
                                Flags);
                            break;
                        }
                        case StringUtils::Hash("ShaderResource"):
                        {
                            Table.AddSrvRange(
                                ShaderRegister,
                                RegisterSpace,
                                RangeCount,
                                Flags);
                            break;
                        }
                        case StringUtils::Hash("UnorderedAccess"):
                        {
                            Table.AddUavRange(
                                ShaderRegister,
                                RegisterSpace,
                                RangeCount,
                                Flags);
                            break;
                        }
                        case StringUtils::Hash("Sampler"):
                        {
                            Table.AddSamplerRange(
                                ShaderRegister,
                                RegisterSpace,
                                RangeCount,
                                Flags);
                            break;
                        }
                        default:
                        {
                            NEON_WARNING("Invalid range type: {}", RangeType);
                        }
                        }
                    }

                    auto Visibility = ConvertShaderVisibility(Parameter.at("Shader Visibility").as_string());
                    Builder.AddDescriptorTable(std::move(Table), Visibility);
                    break;
                }
                case StringUtils::Hash("Constants"):
                {
                    auto ShaderRegister = uint32_t(Parameter.at("Shader Register").as_int64());
                    auto RegisterSpace  = uint32_t(Parameter.at("Register Space").as_int64());
                    auto Count32Bits    = uint32_t(Parameter.at("Count 32 Bits").as_int64());
                    auto Visibility     = ConvertShaderVisibility(Parameter.at("Shader Visibility").as_string());

                    Builder.Add32BitConstants(
                        ShaderRegister,
                        RegisterSpace,
                        Count32Bits,
                        Visibility);
                    break;
                }

                case StringUtils::Hash("Descriptor"):
                {
                    auto ShaderRegister = uint32_t(Parameter.at("Shader Register").as_int64());
                    auto RegisterSpace  = uint32_t(Parameter.at("Register Space").as_int64());
                    auto Visibility     = ConvertShaderVisibility(Parameter.at("Shader Visibility").as_string());
                    auto RootType       = StringU8(Parameter.at("Root Type").as_string());
                    auto Flags          = GetRootFlags(Parameter.at("Flags").as_array());

                    switch (StringUtils::Hash(RootType))
                    {
                    case StringUtils::Hash("ConstantBuffer"):
                    {
                        Builder.AddConstantBufferView(
                            ShaderRegister,
                            RegisterSpace,
                            Visibility,
                            Flags);
                        break;
                    }
                    case StringUtils::Hash("ShaderResource"):
                    {
                        Builder.AddShaderResourceView(
                            ShaderRegister,
                            RegisterSpace,
                            Visibility,
                            Flags);
                        break;
                    }

                    case StringUtils::Hash("UnorderedAccess"):
                    {
                        Builder.AddUnorderedAccessView(
                            ShaderRegister,
                            RegisterSpace,
                            Visibility,
                            Flags);
                        break;
                    }
                    default:
                        NEON_WARNING("Invalid range type: {}", RootType);
                        break;
                    }
                    break;
                }

                default:
                    NEON_WARNING("Invalid root parameter type: {}", ParamType);
                    break;
                }
            }
        }

        if (auto Samplers = Object.find("Static Sampler"); Samplers != Object.end())
        {
            for (auto& Parameter : Samplers->value().as_array() |
                                       std::views::transform([](auto&& Parameter) -> decltype(auto)
                                                             { return Parameter.as_object(); }))
            {
                RHI::StaticSamplerDesc Desc;

                Desc.ShaderRegister = uint32_t(Parameter.at("Shader Register").as_int64());
                Desc.RegisterSpace  = uint32_t(Parameter.at("Register Space").as_int64());
                Desc.Visibility     = ConvertShaderVisibility(Parameter.at("Shader Visibility").as_string());

                Desc.Filter   = ConvertSamplerFilter(StringU8(Parameter.at("Filter").as_string()));
                Desc.AddressU = ConvertSamplerAddressMode(StringU8(Parameter.at("Address U").as_string()));
                Desc.AddressV = ConvertSamplerAddressMode(StringU8(Parameter.at("Address V").as_string()));
                Desc.AddressW = ConvertSamplerAddressMode(StringU8(Parameter.at("Address W").as_string()));

                Desc.MipLODBias     = float(Parameter.at("Mip LOD Bias").as_double());
                Desc.MaxAnisotropy  = uint32_t(Parameter.at("Max Anisotropy").as_int64());
                Desc.ComparisonFunc = ConvertCompareFunc(StringU8(Parameter.at("Comparaison Function").as_string()));
                auto BorderColor    = Parameter.at("Border Color").as_array() |
                                   std::views::transform([](auto&& Color)
                                                         { return float(Color.as_double()); }) |
                                   std::ranges::to<std::vector<float>>();

                Desc.MinLOD = float(Parameter.at("Min LOD").as_double());
                Desc.MaxLOD = float(Parameter.at("Max LOD").as_double());

                NEON_VALIDATE(BorderColor.size() == 4, "Invalid border color");
                Desc.BorderColor = {
                    BorderColor[0],
                    BorderColor[1],
                    BorderColor[2],
                    BorderColor[3]
                };

                Builder.AddSampler(std::move(Desc));
            }
        }

        return std::make_shared<Asset::RootSignatureAsset>(
            Ptr<RHI::IRootSignature>(RHI::IRootSignature::Create(Builder)));
    }
} // namespace PakC::Handler