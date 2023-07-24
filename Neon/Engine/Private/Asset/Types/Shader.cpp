#include <EnginePCH.hpp>
#include <Resource/Types/Shader.hpp>
#include <IO/Archive.hpp>
#include <RHI/Shader.hpp>

#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>

#include <execution>
#include <filesystem>
#include <Core/SHA256.hpp>
#include <Log/Logger.hpp>

namespace ranges = std::ranges;
namespace bio    = boost::iostreams;

namespace Neon::Asset
{
    static StringU8 DecompressString(
        StringU8 Data)
    {
        std::istringstream Stream(std::move(Data));

        bio::filtering_istreambuf Filter;
        Filter.push(bio::gzip_decompressor{});
        Filter.push(Stream);

        std::ostringstream Output;
        bio::copy(Filter, Output);
        return Output.str();
    }

    //

    /// <summary>
    /// Get hash of shader
    /// </summary>
    [[nodiscard]] static SHA256::Bytes GetShaderHash(
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

    Ptr<RHI::IShader> ShaderModule::LoadStage(
        RHI::ShaderStage               Stage,
        const RHI::MShaderCompileFlags Flags,
        RHI::ShaderProfile             Profile,
        const RHI::ShaderMacros&       Macros)
    {
        auto Hash = GetShaderHash(Stage, Flags, Profile, Macros);

        std::scoped_lock Lock(m_ModuleAccessMutex);
        auto             Iter = m_Binaries.find(Hash);
        // Check if shader was already compiled and is in cache
        if (Iter != m_Binaries.end())
        {
            return Iter->second;
        }

        std::unique_ptr<uint8_t[]> ShaderData;
        size_t                     ShaderSize = 0;

        // Check if shader was already compiled
        if (SeekShader(
                Hash,
                &ShaderData,
                &ShaderSize))
        {
            Ptr Shader = RHI::IShader::Create(std::move(ShaderData), ShaderSize);
            m_Binaries.emplace(Hash, Shader);
            return Shader;
        }

        Ptr Shader = RHI::IShader::Create(RHI::ShaderCompileDesc{
            .Macros     = Macros,
            .SourceCode = m_Code,
            .Profile    = Profile,
            .Stage      = Stage,
            .Flags      = Flags,
        });

        if (Shader)
        {
            m_Binaries.emplace(Hash, Shader);
            WriteCache(Hash, Shader.get());
        }

        return Shader;
    }

    void ShaderModule::Optimize()
    {
        std::scoped_lock Lock(m_ModuleAccessMutex);
        m_Binaries.clear();
    }

    StringU8 ShaderModule::Decompress(
        StringU8 Code,
        bool     Compressed)
    {
        if (Compressed)
        {
            std::istringstream Stream(std::move(Code));

            bio::filtering_istreambuf Filter;
            Filter.push(bio::gzip_decompressor{});
            Filter.push(Stream);

            std::ostringstream Output;
            bio::copy(Filter, Output);

            Code = Output.str();
        }
        return std::move(Code);
    }

    ShaderModule::ShaderModule(
        StringU8                ModName,
        StringU8                ModCode,
        bool                    Compressed,
        Ptr<ShaderLibraryAsset> Library,
        ShaderModuleId          Id) :
        m_Library(std::move(Library)),
        m_Id(Id),
        m_Code(Decompress(std::move(ModCode), Compressed))
    {
        auto CachePath = StringUtils::Format("{}_{}.nsmc", std::filesystem::temp_directory_path().string(), ModName);
        m_ShaderCache.open(CachePath, std::ios::in | std::ios::out | std::ios::app | std::ios::binary);

        NEON_ASSERT(m_ShaderCache.is_open(), "Failed to open shader cache file");
        m_ShaderCache.seekg(0, std::ios::end);
        m_FileSize = m_ShaderCache.tellg();

        // Check if the current file will be used for caching
        // The file is used for caching if it exists and has same code as the module
        SHA256 Hash;
        Hash.Append(m_Code);
        auto ExpectedHash = Hash.Digest();

        // File wasn't empty
        if (m_FileSize)
        {
            m_ShaderCache.seekg(0, std::ios::beg);
            SHA256::Bytes CurHash{};
            m_ShaderCache.read(std::bit_cast<char*>(CurHash.data()), CurHash.size());

            if (CurHash == ExpectedHash)
            {
                return;
            }

            m_ShaderCache.close();
            m_ShaderCache.open(CachePath, std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
        }

        m_ShaderCache.write(std::bit_cast<char*>(ExpectedHash.data()), ExpectedHash.size());
        m_FileSize = m_ShaderCache.tellg();
    }

    bool ShaderModule::SeekShader(
        const SHA256::Bytes&,
        std::unique_ptr<uint8_t[]>*,
        size_t*)
    {
        // TODO: Temporarily disabled
#if 0
        m_ShaderCache.seekg(sizeof(SHA256::Bytes), std::ios::beg);

        for (size_t i = sizeof(SHA256::Bytes); i != m_FileSize; i = m_ShaderCache.tellg())
        {
            SHA256::Bytes CurHash;
            m_ShaderCache.read(std::bit_cast<char*>(CurHash.data()), CurHash.size());

            if (Hash == CurHash)
            {
                if (ShaderSize)
                {
                    m_ShaderCache.read(std::bit_cast<char*>(ShaderSize), sizeof(*ShaderSize));

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
#else

        return false;
#endif
    }

    void ShaderModule::WriteCache(
        const SHA256::Bytes& Hash,
        RHI::IShader*        Shader)
    {
#if 0
        m_ShaderCache.seekg(sizeof(SHA256::Bytes), std::ios::beg);

        auto WriteToFile =
            [this, Shader]()
        {
            auto ByteCode = Shader->GetByteCode();
            m_ShaderCache.write(std::bit_cast<char*>(&ByteCode.Size), sizeof(ByteCode.Size));
            m_ShaderCache.write(std::bit_cast<char*>(ByteCode.Data), ByteCode.Size);
        };

        for (size_t i = sizeof(SHA256::Bytes); i != m_FileSize; i = m_ShaderCache.tellg())
        {
            SHA256::Bytes CurHash;
            m_ShaderCache.read(std::bit_cast<char*>(CurHash.data()), CurHash.size());

            if (CurHash == Hash)
            {
                m_ShaderCache.seekp(i, std::ios::beg);
                WriteToFile();
                return;
            }
            else
            {
                uint64_t ShaderSize = 0;
                m_ShaderCache.read(std::bit_cast<char*>(&ShaderSize), sizeof(ShaderSize));
                m_ShaderCache.seekg(ShaderSize, std::ios::cur);
            }
        }

        m_ShaderCache.seekp(0, std::ios::end);
        m_ShaderCache.write(std::bit_cast<char*>(Hash.data()), Hash.size());

        WriteToFile();
#endif
    }

    //

    Ptr<ShaderModule> ShaderLibraryAsset::LoadModule(
        ShaderModuleId Id)
    {
        std::scoped_lock Lock(m_LibraryAccessMutex);

        auto Iter = m_Modules.find(Id);
        return Iter != m_Modules.end() ? Iter->second.Module : nullptr;
    }

    void ShaderLibraryAsset::SetModule(
        ShaderModuleId Id,
        StringU8       ModName,
        StringU8       ModCode)
    {
        std::scoped_lock Lock(m_LibraryAccessMutex);
        SetModule(Id, std::move(ModName), std::move(ModCode), false);
    }

    void ShaderLibraryAsset::RemoveModule(
        ShaderModuleId Id)
    {
        std::scoped_lock Lock(m_LibraryAccessMutex);
        m_Modules.erase(Id);
    }

    void ShaderLibraryAsset::SetModule(
        ShaderModuleId Id,
        StringU8       ModName,
        StringU8       ModCode,
        bool           Compressed)
    {
        m_Modules.erase(Id);
        m_Modules.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(Id),
            std::forward_as_tuple(
                Id,
                std::move(ModName),
                std::move(ModCode),
                Compressed,
                std::static_pointer_cast<ShaderLibraryAsset>(shared_from_this())));
    }

    //

    bool ShaderLibraryAsset::Handler::CanCastTo(
        const Ptr<IAssetResource>& Resource)
    {
        return dynamic_cast<ShaderLibraryAsset*>(Resource.get());
    }

    Ptr<IAssetResource> ShaderLibraryAsset::Handler::Load(
        IAssetPack*,
        IO::InArchive& Archive,
        size_t)
    {
        auto ShaderLib = std::make_shared<ShaderLibraryAsset>();

        size_t NumModules = 0;
        Archive >> NumModules;

        for (size_t i = 0; i < NumModules; i++)
        {
            ShaderModuleId Id;
            Archive >> Id;

            StringU8 ModName;
            Archive >> ModName;

            StringU8 ModCode;
            Archive >> ModCode;

            ShaderLib->SetModule(Id, std::move(ModName), std::move(ModCode), true);
        }

        return ShaderLib;
    }

    void ShaderLibraryAsset::Handler::Save(
        IAssetPack*,
        const Ptr<IAssetResource>& Resource,
        IO::OutArchive&            Archive)
    {
        auto ShaderLib = std::static_pointer_cast<ShaderLibraryAsset>(Resource);
        Archive << ShaderLib->m_Modules.size();

        std::istringstream Stream;
        std::ostringstream CompressedStream;

        bio::filtering_istreambuf Filter;
        Filter.push(bio::gzip_compressor{});
        Filter.push(Stream);

        auto ResetStream = [](auto& Stream)
        {
            Stream.str("");
            Stream.clear();
        };

        for (auto& [ModId, ModData] : ShaderLib->m_Modules)
        {
            Archive << ModId;
            Archive << ModData.ModName;

            ResetStream(Stream);
            ResetStream(CompressedStream);

            Stream.str(ModData.Module->m_Code);
            bio::copy(Filter, CompressedStream);

            Archive << CompressedStream.str();
        }
    }
} // namespace Neon::Asset