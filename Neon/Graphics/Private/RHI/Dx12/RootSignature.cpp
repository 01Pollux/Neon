#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Resource/Common.hpp>
#include <Private/RHI/Dx12/RootSignature.hpp>
#include <Private/RHI/Dx12/Device.hpp>
#include <Private/RHI/Dx12/Shader.hpp>

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
        const wchar_t*          Name,
        const void*             BlobData,
        size_t                  BlobSize,
        Crypto::Sha256::Bytes&& Hash) :
        m_Hash(std::move(Hash))
    {
        auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();
        ThrowIfFailed(Dx12Device->CreateRootSignature(
            0,
            BlobData,
            BlobSize,
            IID_PPV_ARGS(&m_RootSignature)));

#ifndef NEON_DIST
        if (Name)
        {
            RenameObject(m_RootSignature.Get(), Name);
        }
#endif
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

        WinAPI::ComPtr<ID3DBlob> SignatureBlob;
        WinAPI::ComPtr<ID3DBlob> ErrorBlob;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC Desc(
            UINT(Result.Parameters.size()),
            Result.Parameters.data(),
            UINT(Result.StaticSamplers.size()),
            Result.StaticSamplers.data(),
            Result.Flags);

        auto  Dx12Device = Dx12RenderDevice::Get()->GetDevice();
        auto& Features   = Dx12RenderDevice::Get()->GetFeatures();

        HRESULT SerializeRes = D3DX12SerializeVersionedRootSignature(
            &Desc,
            Features.GetRootSignatureVersion(),
            &SignatureBlob,
            &ErrorBlob);

        NEON_VALIDATE(SUCCEEDED(SerializeRes), static_cast<const char*>(ErrorBlob->GetBufferPointer()));

        const void* BlobData = SignatureBlob->GetBufferPointer();
        size_t      BlobSize = SignatureBlob->GetBufferSize();

        auto Digest = Crypto::Sha256().Append(BlobData, BlobSize).Digest();

        std::scoped_lock Lock(s_RootSignatureCacheMutex);

        auto& Cache = s_RootSignatureCache[Digest];
        if (!Cache)
        {
            auto& Name    = Builder.GetName();
            auto  NamePtr = Name.empty() ? nullptr : Name.c_str();
            Cache         = std::make_shared<Dx12RootSignature>(NamePtr, BlobData, BlobSize, std::move(Digest));
        }

        return Cache;
    }

    //

    Ptr<IRootSignature> Dx12RootSignatureCache::Compile(
        const wchar_t*                Name,
        std::span<const Ptr<IShader>> Shaders)
    {
        IShader* FirstShader = nullptr;
        for (auto& Shader : Shaders)
        {
            if (FirstShader = Shader.get())
            {
                break;
            }
        }
        NEON_ASSERT(FirstShader, "No shader was set to compile");

        auto Bytecode = FirstShader->GetByteCode();
        auto Digest   = Crypto::Sha256().Append(Bytecode.Data, Bytecode.Size).Digest();

        std::scoped_lock Lock(s_RootSignatureCacheMutex);

        auto& Cache = s_RootSignatureCache[Digest];
        if (Cache)
        {
            Cache = std::make_shared<Dx12RootSignature>(Name, Bytecode.Data, Bytecode.Size, std::move(Digest));
        }
        return Cache;
    }

    //

    auto Dx12RootSignatureCache::Build(
        const RootSignatureBuilder& Builder) -> BuildResult
    {
        BuildResult Result;
        Result.Flags = CastRootSignatureFlags(Builder.GetFlags());

        auto& NParameters = Builder.GetParameters();
        Result.Parameters.reserve(NParameters.size());
        for (auto& Param : NParameters)
        {
            auto Visibility = CastShaderVisibility(Param.Visibility());

            boost::apply_visitor(
                VariantVisitor{
                    [&Result, Visibility](const RootParameter::DescriptorTable& Table)
                    {
                        auto& Ranges = Result.RangesList.emplace_back();

                        auto& NRanges = Table.GetRanges();
                        Ranges.reserve(NRanges.size());
                        for (auto& Range : NRanges)
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

                        Result.Parameters.emplace_back().InitAsDescriptorTable(UINT(Ranges.size()), Ranges.data(), Visibility);
                    },
                    [&Result, Visibility](const RootParameter::Constants& Constants)
                    {
                        Result.Parameters.emplace_back().InitAsConstants(
                            Constants.Num32BitValues,
                            Constants.ShaderRegister,
                            Constants.RegisterSpace,
                            Visibility);
                    },
                    [&Result, Visibility](const RootParameter::Root& Descriptor)
                    {
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
        for (auto& Sampler : Nsamplers)
        {
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
            else if (CompareColor(1.f))
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

        return Result;
    }
} // namespace Neon::RHI