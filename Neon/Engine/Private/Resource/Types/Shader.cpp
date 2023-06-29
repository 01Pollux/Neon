#include <EnginePCH.hpp>
#include <Resource/Types/Shader.hpp>
#include <RHI/Shader.hpp>

#include <filesystem>
#include <Core/SHA256.hpp>
#include <Log/Logger.hpp>

namespace Neon::Asset
{
    ShaderAsset::ShaderAsset(
        const StringU8& CacheName,
        const StringU8& Code) :
        m_ShaderCacheName(CacheName)
    {
        m_ShaderCache.open(GetTempFileName(), std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
        SetCode(Code);
    }

    const StringU8& ShaderAsset::GetCode() const
    {
        return m_ShaderCode;
    }

    void ShaderAsset::SetCode(
        const StringU8& Code)
    {
        m_ShaderCode = Code;
        // Reset everything in the file
        m_ShaderCache.close();
        m_ShaderCache.open(GetTempFileName(), std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
    }

    /// <summary>
    /// Get hash of shader
    /// </summary>
    [[nodiscard]] SHA256::Bytes GetShaderHash(
        RHI::ShaderStage               Stage,
        const RHI::MShaderCompileFlags Flags   = RHI::MShaderCompileFlags_Default,
        RHI::ShaderProfile             Profile = RHI::ShaderProfile::SP_6_5,
        const RHI::ShaderMacros&       Macros  = {})
    {
        SHA256 Hash;
        Hash.Append(Stage);
        Hash.Append(Flags.ToUllong());
        Hash.Append(Profile);
        for (auto& [Key, Value] : Macros.Defines)
        {
            Hash.Append(Key);
            Hash.Append(Value);
        }
        return Hash.Digest();
    }

    void ShaderAsset::PreloadShader(
        RHI::ShaderStage               Stage,
        const RHI::MShaderCompileFlags Flags,
        RHI::ShaderProfile             Profile,
        const RHI::ShaderMacros&       Macros)
    {
        auto Hash = GetShaderHash(Stage, Flags, Profile, Macros);
    }

    //

    /**
     * Shader cache file format:
     *
     * Hash (32 bytes)
     * Shader size (8 bytes)
     * Shader data (Shader size bytes)
     *
     * Repeat for each shader
     */
    Ptr<RHI::IShader> ShaderAsset::LoadShader(
        RHI::ShaderStage               Stage,
        const RHI::MShaderCompileFlags Flags,
        RHI::ShaderProfile             Profile,
        const RHI::ShaderMacros&       Macros)
    {
    }

    std::filesystem::path ShaderAsset::GetTempFileName() const
    {
        return std::filesystem::path(
            StringUtils::Format(
                "{}_{}.nsc",
                std::filesystem::temp_directory_path().string(),
                m_ShaderCacheName));
    }

    bool ShaderAsset::SeekShader(
        const SHA256::Bytes&        Hash,
        std::unique_ptr<uint8_t[]>* ShaderData,
        size_t*                     ShaderSize)
    {
        m_ShaderCache.seekg(0, std::ios::beg);
        for (size_t i = 0; i < m_FileSize; i = m_ShaderCache.tellg())
        {
            SHA256::Bytes CurHash;
            m_ShaderCache.read(std::bit_cast<char*>(CurHash.data()), CurHash.size());

            if (Hash == Hash)
            {
                if (ShaderSize)
                {
                    m_ShaderCache.read(std::bit_cast<char*>(*ShaderSize), sizeof(*ShaderSize));

                    *ShaderData = std::make_unique<uint8_t[]>(*ShaderSize);
                    m_ShaderCache.read(std::bit_cast<char*>(ShaderData->get()), *ShaderSize);

                    return true;
                }
            }
            else
            {
                uint64_t ShaderSize = 0;
                m_ShaderCache.read(std::bit_cast<char*>(&ShaderSize), sizeof(ShaderSize));

                m_ShaderCache.seekg(ShaderSize, std::ios::cur);
            }
        }
        return false;
    }

    Ptr<RHI::IShader> ShaderAsset::LoadShaderImpl(
        bool                           CreateIfExists,
        RHI::ShaderStage               Stage,
        const RHI::MShaderCompileFlags Flags,
        RHI::ShaderProfile             Profile,
        const RHI::ShaderMacros&       Macros)
    {
        NEON_ASSERT(!m_ShaderCode.empty(), "Shader code is empty");
        NEON_ASSERT(m_ShaderCache.is_open(), "Shader cache file is not open");

        // Check if shader is already compiled
        auto Hash = GetShaderHash(Stage, Flags, Profile, Macros);

        std::unique_ptr<uint8_t[]> ShaderData;
        size_t                     ShaderSize = 0;
        if (SeekShader(Hash, &ShaderData, &ShaderSize))
        {
            if (CreateIfExists)
            {
            }
            return RHI::IShader::Create(std::move(ShaderData), ShaderSize);
        }

        RHI::ShaderCompileDesc Desc{
            .Macros     = Macros,
            .SourceCode = m_ShaderCode,
            .Profile    = Profile,
            .Stage      = Stage,
            .Flags      = Flags,
        };
        auto Shader = RHI::IShader::Create(Desc);

        // Save shader to cache
        m_ShaderCache.seekp(0, std::ios::end);
        m_ShaderCache.write(std::bit_cast<const char*>(Hash.data()), Hash.size());

        auto CodeByte = Shader->GetByteCode();

        ShaderSize = CodeByte.Size;
        m_ShaderCache.write(std::bit_cast<const char*>(&ShaderSize), sizeof(ShaderSize));
        m_ShaderCache.write(std::bit_cast<const char*>(CodeByte.Data), CodeByte.Size);

        m_FileSize = m_ShaderCache.tellp();

        return Shader;
    }

    // ShaderAsset::ShaderAsset(
    //     const Ptr<RHI::IShader>& Shader) :
    //     m_Shader(Shader)
    //{
    // }

    // const Ptr<RHI::IShader>& ShaderAsset::GetShader() const
    //{
    //     return m_Shader;
    // }

    // void ShaderAsset::SetShader(
    //     const Ptr<RHI::IShader>& Shader)
    //{
    //     m_Shader = Shader;
    // }

    ////

    // bool ShaderAsset::Handler::CanCastTo(
    //     const Ptr<IAssetResource>& Resource)
    //{
    //     return dynamic_cast<ShaderAsset*>(Resource.get());
    // }

    // Ptr<IAssetResource> ShaderAsset::Handler::Load(
    //     IAssetPack*,
    //     IO::InArchive& Archive,
    //     size_t         DataSize)
    //{
    //     auto Data(std::make_unique<uint8_t[]>(DataSize));
    //     Archive.load_binary(Data.get(), DataSize);
    //     return std::make_shared<ShaderAsset>(
    //         Ptr<RHI::IShader>(RHI::IShader::Create({ Data.get(), DataSize })));
    // }

    // void ShaderAsset::Handler::Save(
    //     IAssetPack*,
    //     const Ptr<IAssetResource>& Resource,
    //     IO::OutArchive&            Archive)
    //{
    //     auto& Shader = static_cast<ShaderAsset*>(Resource.get())->GetShader();
    //     if (Shader)
    //     {
    //         auto ByteCode = Shader->GetByteCode();
    //         Archive.save_binary(ByteCode.Data, ByteCode.Size);
    //     }
    // }
} // namespace Neon::Asset