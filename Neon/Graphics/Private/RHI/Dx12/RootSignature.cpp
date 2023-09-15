#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Resource/Common.hpp>
#include <Private/RHI/Dx12/RootSignature.hpp>
#include <Private/RHI/Dx12/Device.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
    std::mutex                                           s_RootSignatureCacheMutex;
    std::map<Crypto::Sha256::Bytes, Ptr<IRootSignature>> s_RootSignatureCache;

    //

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

    //

    [[nodiscard]] static constexpr auto GetRootDescriptorFlagList() noexcept
    {
        return std::array{
            std::pair{ D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE, ERootDescriptorFlags::Data_Volatile },
            std::pair{ D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE, ERootDescriptorFlags::Data_Static_While_Execute },
            std::pair{ D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, ERootDescriptorFlags::Data_Static },
        };
    }

    //

    [[nodiscard]] static constexpr auto GetRooSignatureFlagsList() noexcept
    {
        return std::array{
            std::pair{ D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT, ERootSignatureBuilderFlags::AllowInputLayout },
            std::pair{ D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS, ERootSignatureBuilderFlags::DenyVSAccess },
            std::pair{ D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS, ERootSignatureBuilderFlags::DenyHSAccess },
            std::pair{ D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS, ERootSignatureBuilderFlags::DenyDSAccess },
            std::pair{ D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS, ERootSignatureBuilderFlags::DenyGSAccess },
            std::pair{ D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS, ERootSignatureBuilderFlags::DenyPSAccess },
            std::pair{ D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT, ERootSignatureBuilderFlags::AllowStreamOutput },
            std::pair{ D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE, ERootSignatureBuilderFlags::LocalRootSignature },
            std::pair{ D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS, ERootSignatureBuilderFlags::DenyAmpAccess },
            std::pair{ D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS, ERootSignatureBuilderFlags::DenyMeshAccess },
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

    D3D12_ROOT_SIGNATURE_FLAGS CastRootSignatureFlags(
        MRootSignatureBuilderFlags Flags)
    {
        D3D12_ROOT_SIGNATURE_FLAGS Res = D3D12_ROOT_SIGNATURE_FLAG_NONE;
        for (auto& State : GetRooSignatureFlagsList())
        {
            if (Flags.Test(State.second))
            {
                Res |= State.first;
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

    //

    Dx12RootSignature::Dx12RootSignature(
        const RootSignatureBuilder&                  Builder,
        const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& SignatureDesc,
        Crypto::Sha256::Bytes&&                      Hash) :
        m_Hash(std::move(Hash))
    {
        WinAPI::ComPtr<ID3DBlob> SignatureBlob;
        WinAPI::ComPtr<ID3DBlob> ErrorBlob;

        auto  Dx12Device = Dx12RenderDevice::Get()->GetDevice();
        auto& Features   = Dx12RenderDevice::Get()->GetFeatures();

        HRESULT Result = D3DX12SerializeVersionedRootSignature(
            &SignatureDesc,
            Features.GetRootSignatureVersion(),
            &SignatureBlob,
            &ErrorBlob);

        NEON_VALIDATE(SUCCEEDED(Result), static_cast<const char*>(ErrorBlob->GetBufferPointer()));

        ThrowIfFailed(Dx12Device->CreateRootSignature(
            0,
            SignatureBlob->GetBufferPointer(),
            SignatureBlob->GetBufferSize(),
            IID_PPV_ARGS(&m_RootSignature)));

#ifndef NEON_DIST
        if (auto& Name = Builder.GetName(); !Name.empty())
        {
            RenameObject(m_RootSignature.Get(), Name.c_str());
        }
#endif

        //

        int RootIndex = 0;
        m_Params.reserve(Builder.GetParameters().size());
        for (auto& Param : Builder.GetParameters())
        {
            boost::apply_visitor(
                VariantVisitor{
                    [this, &RootIndex](const RootParameter::DescriptorTable& Table)
                    {
                        m_Params.reserve(m_Params.size() + Table.GetRanges().size());
                        for (auto& [Name, Range] : Table.GetRanges())
                        {
                            auto& FinalParam     = m_Params.emplace_back();
                            FinalParam.RootIndex = RootIndex;
                            FinalParam.Name      = Name;
                            FinalParam.Type      = IRootSignature::ParamType::DescriptorTable;

                            FinalParam.Descriptor.Size      = Range.DescriptorCount;
                            FinalParam.Descriptor.Type      = Range.Type;
                            FinalParam.Descriptor.Instanced = Range.Instanced;

                            if (m_ParamMap.contains(Name))
                            {
                                NEON_WARNING_TAG("Root signature parameter name is not unique: {}", Name);
                            }
                            else
                            {
                                m_ParamMap[Name] = m_Params.size() - 1;
                            }
                        }
                    },
                    [this, &RootIndex](const RootParameter::Constants& Constants)
                    {
                        auto& FinalParam     = m_Params.emplace_back();
                        FinalParam.RootIndex = RootIndex;
                        FinalParam.Name      = Constants.Name;
                        FinalParam.Type      = IRootSignature::ParamType::Constants;

                        FinalParam.Constants.Num32BitValues = Constants.Num32BitValues;

                        if (m_ParamMap.contains(Constants.Name))
                        {
                            NEON_WARNING_TAG("Root signature parameter name is not unique: {}", Constants.Name);
                        }
                        else
                        {
                            m_ParamMap[Constants.Name] = m_Params.size() - 1;
                        }
                    },
                    [this, &RootIndex](const RootParameter::Root& RootParam)
                    {
                        auto& FinalParam     = m_Params.emplace_back();
                        FinalParam.RootIndex = RootIndex;
                        FinalParam.Name      = RootParam.Name;
                        FinalParam.Type      = IRootSignature::ParamType::Root;

                        FinalParam.Root.Type = RootParam.Type;

                        if (m_ParamMap.contains(RootParam.Name))
                        {
                            NEON_WARNING_TAG("Root signature parameter name is not unique: {}", RootParam.Name);
                        }
                        else
                        {
                            m_ParamMap[RootParam.Name] = m_Params.size() - 1;
                        }
                    } },
                Param.GetParameter());
            RootIndex++;
        }
    }

    ID3D12RootSignature* Dx12RootSignature::Get()
    {
        return m_RootSignature.Get();
    }

    const Crypto::Sha256::Bytes& Dx12RootSignature::GetHash() const
    {
        return m_Hash;
    }

    //

    void Dx12RootSignatureCache::Flush()
    {
        std::scoped_lock Lock(s_RootSignatureCacheMutex);
        s_RootSignatureCache.clear();
    }

    Ptr<IRootSignature> Dx12RootSignatureCache::Load(
        const RootSignatureBuilder& Builder)
    {
        auto Result = Dx12RootSignatureCache::Build(Builder);

        std::scoped_lock Lock(s_RootSignatureCacheMutex);

        auto& Cache = s_RootSignatureCache[Result.Digest];
        if (!Cache)
        {
            CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC Desc(
                UINT(Result.Parameters.size()),
                Result.Parameters.data(),
                UINT(Result.StaticSamplers.size()),
                Result.StaticSamplers.data(),
                Result.Flags);

            Cache = std::make_shared<Dx12RootSignature>(Builder, Desc, std::move(Result.Digest));
        }

        return Cache;
    }

    //

    auto Dx12RootSignatureCache::Build(
        const RootSignatureBuilder& Builder) -> BuildResult
    {
        BuildResult    Result;
        Crypto::Sha256 Hash;

        auto Flags = CastRootSignatureFlags(Builder.GetFlags());
        Hash << Flags;

        auto& NParameters = Builder.GetParameters();
        Result.Parameters.reserve(NParameters.size());
        for (auto& Param : NParameters)
        {
            auto Visibility = CastShaderVisibility(Param.Visibility());

            boost::apply_visitor(
                VariantVisitor{
                    [&Hash, &Result, Visibility](const RootParameter::DescriptorTable& Table)
                    {
                        auto& Ranges = Result.RangesList.emplace_back();

                        auto& NRanges = Table.GetRanges();
                        Ranges.reserve(NRanges.size());
                        for (auto& Range : NRanges | std::views::values)
                        {
                            D3D12_DESCRIPTOR_RANGE_TYPE Type;

                            switch (Range.Type)
                            {
                            case DescriptorTableParam::ConstantBuffer:
                                Type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                                break;
                            case DescriptorTableParam::ShaderResource:
                                Type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                                break;
                            case DescriptorTableParam::UnorderedAccess:
                                Type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                                break;
                            case DescriptorTableParam::Sampler:
                                Type = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                                break;
                            default:
                                std::unreachable();
                            }

                            // Instanced will allocate unbounded descriptors
                            auto Flags = CastRootDescriptorTableFlags(Range.Flags);

                            uint32_t DescriptorCount;
                            if (Range.Instanced)
                            {
                                DescriptorCount = UINT32_MAX;
                                Flags |= D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
                                Flags &= ~D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_STATIC_KEEPING_BUFFER_BOUNDS_CHECKS;
                            }
                            else
                            {
                                DescriptorCount = Range.DescriptorCount;
                            }

                            Ranges.emplace_back().Init(
                                Type,
                                DescriptorCount,
                                Range.ShaderRegister,
                                Range.RegisterSpace,
                                Flags,
                                Range.Offset);
                        }

                        Hash.Append(std::bit_cast<uint8_t*>(Ranges.data()), sizeof(Ranges[0]) * Ranges.size());
                        Result.Parameters.emplace_back().InitAsDescriptorTable(UINT(Ranges.size()), Ranges.data(), Visibility);
                    },
                    [&Hash, &Result, Visibility](const RootParameter::Constants& Constants)
                    {
                        Hash << Constants.ShaderRegister << Constants.RegisterSpace << Constants.Num32BitValues;
                        Result.Parameters.emplace_back().InitAsConstants(
                            Constants.Num32BitValues,
                            Constants.ShaderRegister,
                            Constants.RegisterSpace,
                            Visibility);
                    },
                    [&Hash, &Result, Visibility](const RootParameter::Root& Descriptor)
                    {
                        Hash << Descriptor.ShaderRegister << Descriptor.RegisterSpace << Descriptor.Type << Descriptor.Flags;
                        auto& Param            = Result.Parameters.emplace_back();
                        Param.ShaderVisibility = Visibility;

                        switch (Descriptor.Type)
                        {
                        case RootParameter::RootType::ConstantBuffer:
                            Param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
                            break;
                        case RootParameter::RootType::ShaderResource:
                            Param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
                            break;
                        case RootParameter::RootType::UnorderedAccess:
                            Param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
                            break;
                        default:
                            std::unreachable();
                        }

                        CD3DX12_ROOT_DESCRIPTOR1::Init(
                            Param.Descriptor,
                            Descriptor.ShaderRegister,
                            Descriptor.RegisterSpace,
                            CastRootDescriptorFlags(Descriptor.Flags));
                    } },
                Param.GetParameter());
        }

        auto& Nsamplers = Builder.GetSamplers();
        Result.StaticSamplers.reserve(Nsamplers.size());
        for (auto& Sampler : Nsamplers | std::views::values)
        {
            Hash.Append(std::bit_cast<uint8_t*>(&Sampler), sizeof(Sampler));

            auto CompareColor = [&Sampler](float Col)
            {
                for (auto Cur : Sampler.BorderColor)
                {
                    if (Cur != Col)
                    {
                        return false;
                    }
                }
                return true;
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

            Result.StaticSamplers.emplace_back(
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

        Result.Digest = Hash.Digest(),
        Result.Flags  = Flags;

        return Result;
    }
} // namespace Neon::RHI