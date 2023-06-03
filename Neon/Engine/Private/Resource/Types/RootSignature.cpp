#include <EnginePCH.hpp>
#include <Resource/Types/RootSignature.hpp>
#include <RHI/RootSignature.hpp>

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
        IO::BinaryStreamReader Stream,
        size_t                 DataSize)
    {
        RHI::RootSignatureBuilder RootSig;
        return nullptr;
    }

    void RootSignatureAsset::Handler::Save(
        const Ptr<IAssetResource>& Resource,
        IO::BinaryStreamWriter     Stream)
    {
    }
} // namespace Neon::Asset