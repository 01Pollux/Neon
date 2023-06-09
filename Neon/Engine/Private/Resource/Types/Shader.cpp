#include <EnginePCH.hpp>
#include <Resource/Types/Shader.hpp>
#include <RHI/Shader.hpp>
#include <IO/Archive.hpp>

namespace Neon::Asset
{
    ShaderAsset::ShaderAsset(
        const Ptr<RHI::IShader>& Shader) :
        m_Shader(Shader)
    {
    }

    const Ptr<RHI::IShader>& ShaderAsset::GetShader() const
    {
        return m_Shader;
    }

    void ShaderAsset::SetShader(
        const Ptr<RHI::IShader>& Shader)
    {
        m_Shader = Shader;
    }

    //

    bool ShaderAsset::Handler::CanCastTo(
        const Ptr<IAssetResource>& Resource)
    {
        return dynamic_cast<ShaderAsset*>(Resource.get());
    }

    Ptr<IAssetResource> ShaderAsset::Handler::Load(
        IAssetPack*,
        IO::InArchive& Archive,
        size_t         DataSize)
    {
        auto Data(std::make_unique<uint8_t[]>(DataSize));
        Archive.load_binary(Data.get(), DataSize);
        return std::make_shared<ShaderAsset>(
            Ptr<RHI::IShader>(RHI::IShader::Create({ Data.get(), DataSize })));
    }

    void ShaderAsset::Handler::Save(
        IAssetPack*,
        const Ptr<IAssetResource>& Resource,
        IO::OutArchive&            Archive)
    {
        auto& Shader = static_cast<ShaderAsset*>(Resource.get())->GetShader();
        if (Shader)
        {
            auto ByteCode = Shader->GetByteCode();
            Archive.save_binary(ByteCode.Data, ByteCode.Size);
        }
    }
} // namespace Neon::Asset