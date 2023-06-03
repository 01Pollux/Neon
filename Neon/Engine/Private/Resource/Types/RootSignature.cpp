#include <EnginePCH.hpp>
#include <Resource/Types/RootSignature.hpp>
#include <RHI/Shader.hpp>

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

    size_t RootSignatureAsset::Handler::QuerySize(
        const Ptr<IAssetResource>& Resource)
    {
        auto& Shader = static_cast<RootSignatureAsset*>(Resource.get())->GetShader();
        return Shader ? Shader->GetByteCode().Size : 0;
    }

    Ptr<IAssetResource> RootSignatureAsset::Handler::Load(
        IO::BinaryStreamReader Stream,
        size_t                 DataSize)
    {
        auto Data(std::make_unique<uint8_t[]>(DataSize));
        Stream.ReadBytes(Data.get(), DataSize);
        return std::make_shared<RootSignatureAsset>(
            Ptr<RHI::IRootSignature>(RHI::IRootSignature::Create({ Data.get(), DataSize })));
    }

    void RootSignatureAsset::Handler::Save(
        const Ptr<IAssetResource>& Resource,
        IO::BinaryStreamWriter     Stream,
        size_t                     DataSize)
    {
        auto& Shader   = static_cast<RootSignatureAsset*>(Resource.get())->GetShader();
        auto  ByteCode = Shader->GetByteCode();
        Stream.WriteBytes(ByteCode.Data, DataSize);
    }
} // namespace Neon::Asset