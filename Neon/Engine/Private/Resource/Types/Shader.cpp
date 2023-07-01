#include <EnginePCH.hpp>
#include <Resource/Types/Shader.hpp>
#include <RHI/Shader.hpp>

#include <cppcoro/resume_on.hpp>
#include <cppcoro/schedule_on.hpp>
#include <cppcoro/sync_wait.hpp>
#include <cppcoro/when_all.hpp>

#include <filesystem>
#include <Core/SHA256.hpp>
#include <Log/Logger.hpp>

namespace ranges = std::ranges;

namespace Neon::Asset
{
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

    RHI::IShader* ShaderModule::LoadStage(
        RHI::ShaderStage               Stage,
        const RHI::MShaderCompileFlags Flags,
        RHI::ShaderProfile             Profile,
        const RHI::ShaderMacros&       Macros)
    {
        auto Hash         = GetShaderHash(Stage, Flags, Profile, Macros);
        using IterMapType = decltype(m_Binaries)::iterator;

        IterMapType Iter;
        {
            std::scoped_lock Lock(m_BinariesMutex);
            Iter = m_Binaries.find(Hash);
        }

        // Check if shader was already compiled and is in cache
        if (Iter != m_Binaries.end())
        {
            return Iter->second.get();
        }

        std::unique_ptr<uint8_t[]> ShaderData;
        size_t                     ShaderSize = 0;

        // Check if shader was already compiled
        if (SeekShader(
                Hash,
                &ShaderData,
                &ShaderSize))
        {
            auto Shader    = RHI::IShader::Create(std::move(ShaderData), ShaderSize);
            auto ShaderPtr = Shader.get();

            {
                std::scoped_lock Lock(m_BinariesMutex);
                m_Binaries.emplace(Hash, std::move(Shader));
            }
            return ShaderPtr;
        }

        auto Shader = RHI::IShader::Create(RHI::ShaderCompileDesc{
            .Macros     = Macros,
            .SourceCode = m_Library->GetModuleCode(m_Id)->c_str(),
            .Profile    = Profile,
            .Stage      = Stage,
            .Flags      = Flags,
        });

        auto ShaderPtr = Shader.get();
        if (Shader)
        {
            {
                std::scoped_lock Lock(m_BinariesMutex);
                m_Binaries.emplace(Hash, std::move(Shader));
            }
            WriteCache(Hash, ShaderPtr);
        }

        return ShaderPtr;
    }

    void ShaderModule::Optimize()
    {
        std::scoped_lock Lock(m_BinariesMutex);
        m_Binaries.clear();
    }

    ShaderModule::ShaderModule(
        const StringU8&     ModName,
        ShaderLibraryAsset* Library,
        ShaderModuleId      Id) :
        m_Library(Library),
        m_Id(Id),
        m_ShaderCache(StringUtils::Format("{}_{}.nsmc", std::filesystem::temp_directory_path().string(), ModName), std::ios::in | std::ios::out | std::ios::app | std::ios::binary)
    {
        NEON_ASSERT(m_ShaderCache.is_open(), "Failed to open shader cache file");
        m_ShaderCache.seekg(0, std::ios::end);
        m_FileSize = m_ShaderCache.tellg();
    }

    bool ShaderModule::SeekShader(
        const SHA256::Bytes&        Hash,
        std::unique_ptr<uint8_t[]>* ShaderData,
        size_t*                     ShaderSize)
    {
        std::scoped_lock Lock(m_ShaderCacheMutex);
        m_ShaderCache.seekg(0, std::ios::beg);

        for (size_t i = 0; i < m_FileSize; i = m_ShaderCache.tellg())
        {
            SHA256::Bytes CurHash;
            m_ShaderCache.read(std::bit_cast<char*>(CurHash.data()), CurHash.size());

            if (Hash == Hash)
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

        return false;
    }

    void ShaderModule::WriteCache(
        const SHA256::Bytes& Hash,
        RHI::IShader*        Shader)
    {
        std::scoped_lock Lock(m_ShaderCacheMutex);

        m_ShaderCache.seekg(0, std::ios::beg);
        m_ShaderCache.seekp(0, std::ios::beg);

        auto WriteToFile =
            [this, Shader]()
        {
            auto ByteCode = Shader->GetByteCode();
            m_ShaderCache.write(std::bit_cast<char*>(&ByteCode.Size), sizeof(ByteCode.Size));
            m_ShaderCache.write(std::bit_cast<char*>(ByteCode.Data), ByteCode.Size);
        };

        for (size_t i = 0; i != m_FileSize; i = m_ShaderCache.tellg())
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
    }

    //

    ShaderModule* ShaderLibraryAsset::LoadModule(
        ShaderModuleId Id)
    {
        auto Iter = m_Modules.find(Id);
        return Iter != m_Modules.end() ? &Iter->second.Module : nullptr;
    }

    const StringU8* ShaderLibraryAsset::GetModuleCode(
        ShaderModuleId Id)
    {
        auto Iter = m_Modules.find(Id);
        if (Iter != m_Modules.end())
        {
            return &*std::next(m_ModulesData.begin(), Iter->second.ModOffset);
        }
        else
        {
            return nullptr;
        }
    }

    void ShaderLibraryAsset::SetModule(
        ShaderModuleId Id,
        StringU8       ModName,
        StringU8       ModCode)
    {
        if (ModName.empty() && ModCode.empty())
        {
            m_Modules.erase(Id);
            return;
        }

        auto Iter = m_Modules.find(Id);
        if (Iter == m_Modules.end())
        {
            m_Modules.emplace(std::piecewise_construct, std::forward_as_tuple(Id), std::forward_as_tuple(Id, ModName, m_ModulesData.size(), ModCode.size(), this));
            m_ModulesData.emplace_back(std::move(ModCode));
            m_ModulesDecompressed.push_back(true);
        }
        else
        {
            size_t Offset = Iter->second.ModOffset;

            *std::next(m_ModulesData.begin(), Offset) = std::move(ModCode);
            m_ModulesDecompressed[Offset]             = true;
        }
    }

    void ShaderLibraryAsset::RemoveModule(
        ShaderModuleId Id)
    {
        auto Iter = m_Modules.find(Id);
        if (Iter != m_Modules.end())
        {
            m_Modules.erase(Iter);
        }
    }

    void ShaderLibraryAsset::Optimize()
    {
        std::vector<cppcoro::task<>> Tasks;
        Tasks.reserve(m_Modules.size());

        for (auto& LoadedData : m_Modules | std::views::values)
        {
            LoadedData.Module.Optimize();
        }
    }
} // namespace Neon::Asset