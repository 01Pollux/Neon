#include <EnginePCH.hpp>
#include <Asset/Handlers/RootSignature.hpp>
#include <RHI/RootSignature.hpp>

#include <boost/serialization/array.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

namespace boost::serialization
{
    template<class _Archive>
    void serialize(
        _Archive&                     Archive,
        Neon::RHI::StaticSamplerDesc& Sampler,
        const unsigned int)
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
        RHI::RootSignatureBuilder RootSignatureBuilder,
        const Handle&             AssetGuid,
        StringU8                  Path) :
        IAsset(AssetGuid, std::move(Path)),
        m_RootSignatureBuilder(std::move(RootSignatureBuilder)),
        m_RootSignature(RHI::IRootSignature::Create(m_RootSignatureBuilder))
    {
    }

    const RHI::RootSignatureBuilder& RootSignatureAsset::GetRootSignatureBuilder() const
    {
        return m_RootSignatureBuilder;
    }

    const Ptr<RHI::IRootSignature>& RootSignatureAsset::GetRootSignature() const
    {
        return m_RootSignature;
    }

    //

    bool RootSignatureAsset::Handler::CanHandle(
        const Ptr<IAsset>& Resource)
    {
        return dynamic_cast<RootSignatureAsset*>(Resource.get());
    }

    Ptr<IAsset> RootSignatureAsset::Handler::Load(
        std::istream& Stream,
        const Asset::DependencyReader&,
        const Handle&        AssetGuid,
        StringU8             Path,
        const AssetMetaData& LoaderData)
    {
        RHI::RootSignatureBuilder RootSig;

        boost::archive::text_iarchive Archive(Stream, boost::archive::no_header | boost::archive::no_tracking);
        Archive >> RootSig;

        return std::make_shared<RootSignatureAsset>(std::move(RootSig), AssetGuid, std::move(Path));
    }

    void RootSignatureAsset::Handler::Save(
        std::iostream& Stream,
        DependencyWriter&,
        const Ptr<IAsset>& Asset,
        AssetMetaData&     LoaderData)
    {
        auto RootSig = static_cast<RootSignatureAsset*>(Asset.get());

        boost::archive::text_oarchive Archive(Stream, boost::archive::no_header | boost::archive::no_tracking);
        Archive << RootSig->GetRootSignatureBuilder();
    }
} // namespace Neon::Asset
