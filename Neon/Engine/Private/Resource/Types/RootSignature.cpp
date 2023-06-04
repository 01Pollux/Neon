#include <EnginePCH.hpp>
#include <Resource/Types/RootSignature.hpp>
#include <RHI/RootSignature.hpp>
#include <IO/Archive.hpp>

namespace Neon::Asset
{
    RootSignatureAsset::RootSignatureAsset(
        const Ptr<RHI::IRootSignature>& RootSignature) :
        m_RootSignature(RootSignature)
    {
    }

    const Ptr<RHI::IRootSignature>& RootSignatureAsset::GetRootSignature() const
    {
        return m_RootSignature;
    }

    void RootSignatureAsset::SetRootSignature(
        const Ptr<RHI::IRootSignature>& RootSignature)
    {
        m_RootSignature = RootSignature;
    }

    //

    enum class RootSignatureSectionType : uint8_t
    {
        DescriptorTable,
        ConstantBufferView,
        ShaderResourceView,
        UnorderedAccesView,
        Sampler
    };

    bool RootSignatureAsset::Handler::CanCastTo(
        const Ptr<IAssetResource>& Resource)
    {
        return dynamic_cast<RootSignatureAsset*>(Resource.get());
    }

    Ptr<IAssetResource> RootSignatureAsset::Handler::Load(
        IO::InArchive& Archive,
        size_t         DataSize)
    {
        RHI::RootSignatureBuilder RootSig;

        // auto Count = Stream.Read<uint32_t>();
        // for (uint32_t i = 0; i < Count; ++i)
        //{
        //     auto Type       = Stream.Read<RootSignatureSectionType>();
        //     auto Visibility = Stream.Read<RHI::ShaderVisibility>();

        //    switch (Type)
        //    {
        //    case RootSignatureSectionType::DescriptorTable:
        //    {
        //        RHI::RootDescriptorTable Table;

        //        uint32_t Ranges = Stream.Read<uint32_t>();
        //        for (uint32_t j = 0; j < Ranges; ++j)
        //        {
        //            auto RangeType = Stream.Read<RHI::DescriptorTableParam>();

        //            auto ShaderRegister  = Stream.Read<uint32_t>();
        //            auto RegisterSpace   = Stream.Read<uint32_t>();
        //            auto DescriptorCount = Stream.Read<uint32_t>();
        //            auto Flags           = RHI::MRootDescriptorTableFlags(Stream.Read<uint32_t>());
        //            switch (RangeType)
        //            {
        //            case RHI::DescriptorTableParam::ConstantBuffer:
        //                Table.AddCbvRange(
        //                    ShaderRegister,
        //                    RegisterSpace,
        //                    DescriptorCount,
        //                    Flags);
        //                break;
        //            case RHI::DescriptorTableParam::ShaderResource:
        //                Table.AddSrvRange(
        //                    ShaderRegister,
        //                    RegisterSpace,
        //                    DescriptorCount,
        //                    Flags);
        //                break;
        //            case RHI::DescriptorTableParam::UnorderedAccess:
        //                Table.AddUavRange(
        //                    ShaderRegister,
        //                    RegisterSpace,
        //                    DescriptorCount,
        //                    Flags);
        //                break;
        //            case RHI::DescriptorTableParam::Sampler:
        //                Table.AddSamplerRange(
        //                    ShaderRegister,
        //                    RegisterSpace,
        //                    DescriptorCount,
        //                    Flags);
        //                break;
        //            default:
        //                break;
        //            }
        //        }

        //        RootSig.AddDescriptorTable(std::move(Table), Visibility);
        //        break;
        //    }

        //    case RootSignatureSectionType::ConstantBufferView:
        //    {
        //        auto ShaderRegister = Stream.Read<uint32_t>();
        //        auto RegisterSpace  = Stream.Read<uint32_t>();
        //        auto Flags          = RHI::MRootDescriptorFlags(Stream.Read<uint32_t>());

        //        RootSig.AddConstantBufferView(
        //            ShaderRegister,
        //            RegisterSpace,
        //            Visibility,
        //            Flags);
        //        break;
        //    }

        //    case RootSignatureSectionType::ShaderResourceView:
        //    {
        //        auto ShaderRegister = Stream.Read<uint32_t>();
        //        auto RegisterSpace  = Stream.Read<uint32_t>();
        //        auto Flags          = RHI::MRootDescriptorFlags(Stream.Read<uint32_t>());

        //        RootSig.AddShaderResourceView(
        //            ShaderRegister,
        //            RegisterSpace,
        //            Visibility,
        //            Flags);
        //        break;
        //    }

        //    case RootSignatureSectionType::UnorderedAccesView:
        //    {
        //        auto ShaderRegister = Stream.Read<uint32_t>();
        //        auto RegisterSpace  = Stream.Read<uint32_t>();
        //        auto Flags          = RHI::MRootDescriptorFlags(Stream.Read<uint32_t>());

        //        RootSig.AddUnorderedAccessView(
        //            ShaderRegister,
        //            RegisterSpace,
        //            Visibility,
        //            Flags);
        //        break;
        //    }

        //    case RootSignatureSectionType::Sampler:
        //    {
        //        /*   auto ShaderRegister = Stream.Read<uint32_t>();
        //           auto RegisterSpace  = Stream.Read<uint32_t>();

        //           auto        Filter         = Stream.Read<RHI::ESamplerFilter>();
        //           auto        AddressU       = Stream.Read<RHI::ESamplerMode>();
        //           auto        AddressV       = Stream.Read<RHI::ESamplerMode>();
        //           auto        AddressW       = Stream.Read<RHI::ESamplerMode>();
        //           auto        MipLODBias     = Stream.Read<float>();
        //           uint32_t    MaxAnisotropy  = 1;
        //           ESamplerCmp ComparisonFunc = ESamplerCmp::Less;
        //           float       BorderColor[4] = { 1.f, 0.f, 1.f, 1.f };
        //           float       MinLOD         = -16.f;
        //           float       MaxLOD         = 15.99f;*/

        //        break;
        //    }

        //    default:
        //        break;
        //    }
        //}

        return nullptr;
    }

    void RootSignatureAsset::Handler::Save(
        const Ptr<IAssetResource>& Resource,
        IO::OutArchive&            Archive)
    {
    }
} // namespace Neon::Asset
