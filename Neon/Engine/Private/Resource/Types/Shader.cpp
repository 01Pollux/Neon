#include <EnginePCH.hpp>
#include <Resource/Types/Shader.hpp>
#include <RHI/Shader.hpp>

namespace Neon::Asset
{
    ShaderAsset::ShaderAsset(
        const Ptr<RHI::IShader>& Shader) :
        m_Shader(Shader)
    {
    }

    const Ptr<RHI::IShader>& ShaderAsset::GetShader()
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

    size_t ShaderAsset::Handler::QuerySize(
        const Ptr<IAssetResource>& Resource)
    {
        auto& Shader = static_cast<ShaderAsset*>(Resource.get())->GetShader();
        return Shader ? Shader->GetByteCode().Size : 0;
    }

    Ptr<IAssetResource> ShaderAsset::Handler::Load(
        std::istream& Stream,
        size_t        DataSize)
    {
        auto Data(std::make_unique<uint8_t[]>(DataSize));
        Stream.read(std::bit_cast<char*>(Data.get()), DataSize);
        return std::make_shared<ShaderAsset>(
            Ptr<RHI::IShader>(RHI::IShader::Create({ Data.get(), DataSize })));
    }

    void ShaderAsset::Handler::Save(
        const Ptr<IAssetResource>& Resource,
        std::ostream&              Stream,
        size_t                     DataSize)
    {
        auto& Shader   = static_cast<ShaderAsset*>(Resource.get())->GetShader();
        auto  ByteCode = Shader->GetByteCode();
        Stream.write(std::bit_cast<char*>(ByteCode.Data), DataSize);
    }
} // namespace Neon::Asset