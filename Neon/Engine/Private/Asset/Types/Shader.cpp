#include <EnginePCH.hpp>
#include <Asset/Handlers/Shader.hpp>
#include <Crypto/SHA256.hpp>
#include <RHI/Shader.hpp>

#include <Log/Logger.hpp>

namespace ranges = std::ranges;

namespace Neon::Asset
{
    /// <summary>
    /// Get the shader hash from the shader compile description.
    /// </summary>
    [[nodiscard]] static Crypto::Sha256::Bytes GetShaderHash(
        const RHI::ShaderCompileDesc& Desc,
        const StringU8&               IncludeDirectory)
    {
        Crypto::Sha256 Hash;
        Hash << Desc.Stage << Desc.Flags.ToUllong() << Desc.Profile;
        for (auto& [Key, Value] : Desc.Macros.Defines)
        {
            Hash << Key << Value;
        }
        return Hash.Digest();
    }

    //

    ShaderAsset::ShaderAsset(
        StringU8      ShaderCode,
        const Handle& AssetGuid,
        StringU8      Path) :
        IAsset(AssetGuid, std::move(Path)),
        m_ShaderCode(std::move(ShaderCode))
    {
        OpenCacheFile(false);
    }

    UPtr<RHI::IShader> ShaderAsset::LoadShader(
        const RHI::ShaderCompileDesc& Desc,
        const StringU8&               IncludeDirectory)
    {
        Crypto::Sha256::Bytes Hash{};

        auto   ShaderHash = GetShaderHash(Desc, IncludeDirectory);
        size_t ShaderSize = 0;

        m_ShaderCacheFile.seekg(sizeof(Crypto::Sha256::Bytes), std::ios::beg);
        // Skip the first hash, which is the header hash.
        for (size_t i = sizeof(Crypto::Sha256::Bytes); i < m_ShaderCacheSize; i = m_ShaderCacheFile.tellg())
        {
            m_ShaderCacheFile.read(std::bit_cast<char*>(Hash.data()), Hash.size());
            m_ShaderCacheFile.read(std::bit_cast<char*>(&ShaderSize), sizeof(ShaderSize));

            if (ShaderHash == Hash)
            {
                auto ShaderData = std::make_unique<uint8_t[]>(ShaderSize);
                m_ShaderCacheFile.read(std::bit_cast<char*>(ShaderData.get()), ShaderSize);

                return RHI::IShader::Create(std::move(ShaderData), ShaderSize);
            }

            m_ShaderCacheFile.seekg(ShaderSize, std::ios::cur);
        }

        auto ShaderData     = RHI::IShader::Create(m_ShaderCode, Desc);
        auto ShaderBytecode = ShaderData->GetByteCode();

        m_ShaderCacheFile.seekp(0, std::ios::end);

        m_ShaderCacheFile.write(std::bit_cast<const char*>(ShaderHash.data()), ShaderHash.size());
        m_ShaderCacheFile.write(std::bit_cast<const char*>(&ShaderBytecode.Size), sizeof(ShaderBytecode.Size));
        m_ShaderCacheFile.write(std::bit_cast<const char*>(ShaderBytecode.Data), ShaderBytecode.Size);

        m_ShaderCacheFile.flush();
        m_ShaderCacheSize = m_ShaderCacheFile.tellp();

        return ShaderData;
    }

    void ShaderAsset::ClearCache()
    {
        OpenCacheFile(true);
    }

    void ShaderAsset::OpenCacheFile(
        bool Reset)
    {
        auto WriteHeaderDigest =
            [this](const Crypto::Sha256::Bytes& Digest)
        {
            m_ShaderCacheFile.seekp(std::ios::beg);
            m_ShaderCacheFile.write(std::bit_cast<const char*>(Digest.data()), Digest.size());
        };

        auto GetShaderCodeDigest = [this]
        {
            Crypto::Sha256 ExpectedHash;
            ExpectedHash << m_ShaderCode;
            return ExpectedHash.Digest();
        };

        auto WriteHeaderDigestFromCode =
            [this,
             WriteHeaderDigest,
             GetShaderCodeDigest]()
        {
            WriteHeaderDigest(GetShaderCodeDigest());
        };

        auto ReadHeaderDigest =
            [this]()
        {
            Crypto::Sha256::Bytes ShaderDigest{};
            m_ShaderCacheFile.seekg(std::ios::beg);
            m_ShaderCacheFile.read(std::bit_cast<char*>(ShaderDigest.data()), ShaderDigest.size());
            return ShaderDigest;
        };

        //

        if (m_ShaderCacheFile.is_open())
        {
            m_ShaderCacheFile.close();
        }

        std::filesystem::path CachePath = StringUtils::Format("{}{}.nsc", std::filesystem::temp_directory_path().string(), GetGuid().ToString());
        std::ios::openmode    Flags     = std::ios::in | std::ios::out | std::ios::binary;

        bool FileExists = std::filesystem::exists(CachePath);

        if (Reset || !FileExists)
        {
            Flags |= std::ios::trunc;
            FileExists = false;
        }

        m_ShaderCacheFile.open(CachePath, Flags);

        if (FileExists)
        {
            auto ShaderDigest   = ReadHeaderDigest();
            auto ExpectedDigest = GetShaderCodeDigest();
            if (ExpectedDigest != ShaderDigest)
            {
                m_ShaderCacheFile.close();
                m_ShaderCacheFile.open(CachePath, Flags | std::ios::trunc);
                WriteHeaderDigestFromCode();
            }
        }
        else
        {
            WriteHeaderDigestFromCode();
        }

        m_ShaderCacheFile.seekg(0, std::ios::end);
        m_ShaderCacheSize = m_ShaderCacheFile.tellg();
    }

    //

    bool ShaderAsset::Handler::CanHandle(
        const Ptr<IAsset>& Resource)
    {
        return dynamic_cast<ShaderAsset*>(Resource.get()) != nullptr;
    }

    Ptr<IAsset> ShaderAsset::Handler::Load(
        std::istream& Stream,
        const Asset::DependencyReader&,
        const Handle&        AssetGuid,
        StringU8             Path,
        const AssetMetaData& LoaderData)
    {
        Stream.seekg(0, std::ios::end);
        auto FileSize = Stream.tellg();
        Stream.seekg(std::ios::beg);

        StringU8 ShaderCode(FileSize, '\0');
        Stream.read(ShaderCode.data(), FileSize);

        return std::make_shared<ShaderAsset>(std::move(ShaderCode), AssetGuid, std::move(Path));
    }

    void ShaderAsset::Handler::Save(
        std::iostream& Stream,
        DependencyWriter&,
        const Ptr<IAsset>& Asset,
        AssetMetaData&     LoaderData)
    {
        auto Shader = static_cast<ShaderAsset*>(Asset.get());
        Stream.write(Shader->m_ShaderCode.data(), Shader->m_ShaderCode.size());
    }
} // namespace Neon::Asset