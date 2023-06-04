#include <EnginePCH.hpp>
#include <Resource/Types/RootSignature.hpp>
#include <RHI/RootSignature.hpp>

#include <boost/serialization/vector.hpp>
#include <IO/Archive.hpp>

namespace boost::serialization
{

    template<class _Archive>
    void serialize(
        _Archive&                     Archive,
        Neon::RHI::StaticSamplerDesc& Sampler,
        const unsigned int            Version)
    {
        Archive& Sampler.Filter;
        Archive& Sampler.AddressU;
        Archive& Sampler.AddressV;
        Archive& Sampler.AddressW;
        Archive& Sampler.MipLODBias;
        Archive& Sampler.MaxAnisotropy;
        Archive& Sampler.ComparisonFunc;
        Archive& Sampler.BorderColor;
        Archive& Sampler.MinLOD;
        Archive& Sampler.MaxLOD;
        Archive& Sampler.ShaderRegister;
        Archive& Sampler.RegisterSpace;

        auto     Visibility = std::to_underlying(Sampler.Visibility);
        Archive& Visibility;
        Sampler.Visibility = static_cast<Neon::RHI::ShaderVisibility>(Visibility);
    }
} // namespace boost::serialization

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
        Archive >> RootSig;
        return std::make_shared<RootSignatureAsset>(RHI::IRootSignature::Create(RootSig));
    }

    void RootSignatureAsset::Handler::Save(
        const Ptr<IAssetResource>& Resource,
        IO::OutArchive&            Archive)
    {
        auto RootSig = std::dynamic_pointer_cast<RootSignatureAsset>(Resource);
        Archive << RootSig->GetRootSignature()->GetBuilder();
    }
} // namespace Neon::Asset
