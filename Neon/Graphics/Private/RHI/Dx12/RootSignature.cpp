#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Resource/Common.hpp>
#include <Private/RHI/Dx12/RootSignature.hpp>
#include <Private/RHI/Dx12/Device.hpp>

#include <execution>
#include <Core/SHA256.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
    [[nodiscard]] static constexpr auto GetRootDescriptorTableFlagList() noexcept
    {
        return std::array{
            std::pair{ D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, ERootDescriptorTableFlags::Descriptor_Volatile },
            std::pair{ D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE, ERootDescriptorTableFlags::Data_Volatile },
            std::pair{ D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE, ERootDescriptorTableFlags::Data_Static_While_Execute },
            std::pair{ D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, ERootDescriptorTableFlags::Data_Static },
            std::pair{ D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_STATIC_KEEPING_BUFFER_BOUNDS_CHECKS, ERootDescriptorTableFlags::Descriptor_Static_Bounds_Check },
        };
    }

    [[nodiscard]] static constexpr auto GetRootDescriptorFlagList() noexcept
    {
        return std::array{
            std::pair{ D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE, ERootDescriptorFlags::Data_Volatile },
            std::pair{ D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE, ERootDescriptorFlags::Data_Static_While_Execute },
            std::pair{ D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, ERootDescriptorFlags::Data_Static },
        };
    }

    //

    D3D12_SHADER_VISIBILITY CastShaderVisibility(
        ShaderVisibility Visibility)
    {
        switch (Visibility)
        {
        case ShaderVisibility::All:
            return D3D12_SHADER_VISIBILITY_ALL;
        case ShaderVisibility::Vertex:
            return D3D12_SHADER_VISIBILITY_VERTEX;
        case ShaderVisibility::Hull:
            return D3D12_SHADER_VISIBILITY_HULL;
        case ShaderVisibility::Domain:
            return D3D12_SHADER_VISIBILITY_DOMAIN;
        case ShaderVisibility::Geometry:
            return D3D12_SHADER_VISIBILITY_GEOMETRY;
        case ShaderVisibility::Pixel:
            return D3D12_SHADER_VISIBILITY_PIXEL;
        default:
            std::unreachable();
        }
        return D3D12_SHADER_VISIBILITY();
    }

    ShaderVisibility CastShaderVisibility(
        D3D12_SHADER_VISIBILITY Visibility)
    {
        switch (Visibility)
        {
        case D3D12_SHADER_VISIBILITY_ALL:
            return ShaderVisibility::All;
        case D3D12_SHADER_VISIBILITY_VERTEX:
            return ShaderVisibility::Vertex;
        case D3D12_SHADER_VISIBILITY_HULL:
            return ShaderVisibility::Hull;
        case D3D12_SHADER_VISIBILITY_DOMAIN:
            return ShaderVisibility::Domain;
        case D3D12_SHADER_VISIBILITY_GEOMETRY:
            return ShaderVisibility::Geometry;
        case D3D12_SHADER_VISIBILITY_PIXEL:
            return ShaderVisibility::Pixel;
        default:
            std::unreachable();
        }
    }

    //

    D3D12_DESCRIPTOR_RANGE_FLAGS CastRootDescriptorTableFlags(
        const MRootDescriptorTableFlags& Flags)
    {
        D3D12_DESCRIPTOR_RANGE_FLAGS Res = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
        for (auto& State : GetRootDescriptorTableFlagList())
        {
            if (Flags.Test(State.second))
            {
                Res |= State.first;
            }
        }
        return Res;
    }

    MRootDescriptorTableFlags CastRootDescriptorTableFlags(
        D3D12_DESCRIPTOR_RANGE_FLAGS Flags)
    {
        MRootDescriptorTableFlags Res;
        for (auto& State : GetRootDescriptorTableFlagList())
        {
            if (Flags & State.first)
            {
                Res.Set(State.second);
            }
        }
        return Res;
    }

    //

    D3D12_ROOT_DESCRIPTOR_FLAGS CastRootDescriptorFlags(
        const MRootDescriptorFlags& Flags)
    {
        D3D12_ROOT_DESCRIPTOR_FLAGS Res = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
        for (auto& State : GetRootDescriptorFlagList())
        {
            if (Flags.Test(State.second))
            {
                Res |= State.first;
            }
        }
        return Res;
    }

    MRootDescriptorFlags CastRootDescriptorFlags(
        D3D12_ROOT_DESCRIPTOR_FLAGS Flags)
    {
        MRootDescriptorFlags Res;
        for (auto& State : GetRootDescriptorFlagList())
        {
            if (Flags & State.first)
            {
                Res.Set(State.second);
            }
        }
        return Res;
    }

    //

    Ptr<IRootSignature> IRootSignature::Create(
        const RootSignatureBuilder& Builder)
    {
        return Dx12RootSignatureCache::Load(Builder);
    }

    Dx12RootSignature::Dx12RootSignature(
        const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& SignatureDesc,
        uint32_t                                     ResourceCount,
        uint32_t                                     SamplerCount)
    {
        Win32::ComPtr<ID3DBlob> SignatureBlob;
        Win32::ComPtr<ID3DBlob> ErrorBlob;

        auto  Dx12Device = Dx12RenderDevice::Get()->GetDevice();
        auto& Features   = Dx12RenderDevice::Get()->GetFeatures();

        HRESULT Result = D3DX12SerializeVersionedRootSignature(
            &SignatureDesc,
            Features.GetRootSignatureVersion(),
            &SignatureBlob,
            &ErrorBlob);

        if (FAILED(Result))
        {
            auto Error = static_cast<const char*>(ErrorBlob->GetBufferPointer());
            NEON_VALIDATE(false, Error);
        }

        ThrowIfFailed(Dx12Device->CreateRootSignature(
            0,
            SignatureBlob->GetBufferPointer(),
            SignatureBlob->GetBufferSize(),
            IID_PPV_ARGS(&m_RootSignature)));
    }

    Dx12RootSignature::~Dx12RootSignature()
    {
        if (m_RootSignature)
        {
            m_RootSignature->Release();
            m_RootSignature = nullptr;
        }
    }

    //

    Ptr<Dx12RootSignature> Dx12RootSignatureCache::Load(
        const RootSignatureBuilder& Builder)
    {
        auto Result = Dx12RootSignatureCache::Build(Builder);

        return Ptr<Dx12RootSignature>();
    }

    auto Dx12RootSignatureCache::Build(
        const RootSignatureBuilder& Builder) -> BuildResult
    {
        std::list<std::vector<CD3DX12_DESCRIPTOR_RANGE1>> RangesList;
        std::vector<CD3DX12_ROOT_PARAMETER1>              Parameters;
        std::vector<CD3DX12_STATIC_SAMPLER_DESC>          StaticSamplers;

        SHA256 Hash;

        Hash.Append(Builder.GetFlags().ToUllong());

        auto& NParameters = Builder.GetParameters();
        Parameters.reserve(NParameters.size());
        for (auto& Param : NParameters)
        {
            auto Visibility = CastShaderVisibility(Param.GetVisibility());

            std::visit(
                VariantVisitor{
                    [&Hash, &Parameters, &RangesList, Visibility](const RootParameter::DescriptorTable& Table)
                    {
                        auto& Ranges = RangesList.emplace_back();

                        auto& NRanges = Table.GetRanges();
                        Ranges.reserve(NRanges.size());
                        for (auto& Range : NRanges)
                        {
                            D3D12_DESCRIPTOR_RANGE_TYPE Type;

                            switch (Range.Type)
                            {
                            case RootParameter::DescriptorType::ConstantBuffer:
                                Type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                                break;
                            case RootParameter::DescriptorType::ShaderResource:
                                Type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                                break;
                            case RootParameter::DescriptorType::UnorderedAccess:
                                Type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                                break;
                            case RootParameter::DescriptorType::Sampler:
                                Type = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                                break;
                            }

                            Ranges.emplace_back().Init(
                                Type,
                                Range.DescriptorCount,
                                Range.ShaderRegister,
                                Range.RegisterSpace,
                                CastRootDescriptorTableFlags(Range.Flags));
                        }

                        Hash.Append(std::bit_cast<uint8_t*>(Ranges.data()), sizeof(Ranges[0]) * Ranges.size());
                        Parameters.emplace_back().InitAsDescriptorTable(Ranges.size(), Ranges.data(), Visibility);
                    },
                    [&Hash, &Parameters, Visibility](const RootParameter::Constants& Constants)
                    {
                        Hash.Append(std::bit_cast<uint8_t*>(&Constants), sizeof(Constants));
                        Parameters.emplace_back().InitAsConstants(
                            Constants.Num32BitValues,
                            Constants.ShaderRegister,
                            Constants.RegisterSpace,
                            Visibility);
                    },
                    [&Hash, &Parameters, Visibility](const RootParameter::Descriptor& Descriptor)
                    {
                        Hash.Append(std::bit_cast<uint8_t*>(&Descriptor), sizeof(Descriptor));
                        auto& Param            = Parameters.emplace_back();
                        Param.ShaderVisibility = Visibility;

                        switch (Descriptor.Type)
                        {
                        case RootParameter::DescriptorType::ConstantBuffer:
                            Param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
                            break;
                        case RootParameter::DescriptorType::ShaderResource:
                            Param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
                            break;
                        case RootParameter::DescriptorType::UnorderedAccess:
                            Param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
                            break;
                        }

                        CD3DX12_ROOT_DESCRIPTOR1::Init(
                            Param.Descriptor,
                            Descriptor.RegisterSpace,
                            Descriptor.RegisterSpace,
                            CastRootDescriptorFlags(Descriptor.Flags));
                    } },
                Param.GetParameter());
        }

        auto& Nsamplers = Builder.GetSamplers();
        StaticSamplers.reserve(Nsamplers.size());
        for (auto& Sampler : Nsamplers)
        {
            Hash.Append(std::bit_cast<uint8_t*>(&Sampler), sizeof(Sampler));

            auto CompareColor = [&Sampler](float Col)
            {
                return std::equal(
                    std::execution::par_unseq,
                    std::begin(Sampler.BorderColor),
                    std::end(Sampler.BorderColor),
                    [Col](float Cur)
                    { return Cur == Col; });
            };
            D3D12_STATIC_BORDER_COLOR BorderColor;
            if (CompareColor(0.f))
            {
                BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
            }
            else if (CompareColor(0.f))
            {
                BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
            }
            else
            {
                BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
            }

            StaticSamplers.emplace_back(
                Sampler.ShaderRegister,
                CastFilter(Sampler.Filter),
                CastAddressMode(Sampler.AddressU),
                CastAddressMode(Sampler.AddressV),
                CastAddressMode(Sampler.AddressW),
                Sampler.MipLODBias,
                Sampler.MaxAnisotropy,
                CastComparisonFunc(Sampler.ComparisonFunc),
                BorderColor,
                Sampler.MinLOD,
                Sampler.MaxLOD,
                CastShaderVisibility(Sampler.Visibility),
                Sampler.RegisterSpace);
        }

        return {
            .Digest         = Hash.Digest(),
            .RangesList     = std::move(RangesList),
            .Parameters     = std::move(Parameters),
            .StaticSamplers = std::move(StaticSamplers),
        };
    }
} // namespace Neon::RHI