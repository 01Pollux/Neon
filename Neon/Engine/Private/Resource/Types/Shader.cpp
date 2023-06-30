#include <EnginePCH.hpp>
#include <Resource/Types/Shader.hpp>
#include <RHI/Shader.hpp>

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

    std::future<RHI::IShader*> ShaderModule::LoadStage(
        RHI::ShaderStage               Stage,
        const RHI::MShaderCompileFlags Flags,
        RHI::ShaderProfile             Profile,
        const RHI::ShaderMacros&       Macros)
    {
        auto Hash = GetShaderHash(Stage, Flags, Profile, Macros);
        auto Iter = m_Binaries.find(Hash);

        if (Iter != m_Binaries.end())
        {
            return std::async(
                std::launch::deferred,
                [this, Iter]
                { return Iter->second.get(); });
        }

        std::unique_ptr<uint8_t[]> ShaderData;
        size_t                     ShaderSize = 0;
        if (SeekShader(
                Hash,
                &ShaderData,
                &ShaderSize))
        {
            auto Shader    = RHI::IShader::Create(std::move(ShaderData), ShaderSize);
            auto ShaderPtr = Shader.get();
            m_Binaries.emplace(Hash, std::move(Shader));

            return std::async(
                std::launch::deferred,
                [ShaderPtr]
                {
                    return ShaderPtr;
                });
        }

        auto Promise = std::make_shared<std::promise<RHI::IShader*>>();
        auto Future  = Promise->get_future();

        m_Library->m_ThreadPool.Enqueue(
            [this, Stage, Flags, Profile, Macros, Hash, Promise]() mutable
            {
                auto Shader = RHI::IShader::Create(
                    RHI::ShaderCompileDesc{
                        .Macros     = Macros,
                        .SourceCode = m_Library->GetModuleCode(m_Id)->c_str(),
                        .Profile    = Profile,
                        .Stage      = Stage,
                        .Flags      = Flags,
                    });

                auto ShaderPtr = Shader.get();
                if (Shader)
                {
                    m_Binaries.emplace(Hash, std::move(Shader));
                    m_Library->m_ThreadPool.Enqueue(
                        [this, Hash = std::move(Hash), ShaderPtr]
                        {
                            WriteCache(Hash, ShaderPtr);
                        });
                }

                Promise->set_value(ShaderPtr);
            });

        return Future;
    }

    void ShaderModule::Optimize()
    {
        NEON_ASSERT(false, "TODO");
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

            if (Hash == Hash)
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
            m_ThreadPool.Wait();
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

    void ShaderLibraryAsset::Optimize()
    {
        for (auto& LoadedData : m_Modules | std::views::values)
        {
            LoadedData.Module.Optimize();
        }
    }

    //

    // ShaderAsset::ShaderAsset(
    //     const StringU8& CacheName,
    //     const StringU8& Code) :
    //     m_ShaderCacheName(CacheName)
    //{
    //     m_ShaderCache.open(GetTempFileName(), std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
    //     SetCode(Code);
    // }

    // const StringU8& ShaderAsset::GetCode() const
    //{
    //     return m_ShaderCode;
    // }

    // void ShaderAsset::SetCode(
    //     const StringU8& Code)
    //{
    //     m_ShaderCode = Code;
    //     // Reset everything in the file
    //     m_ShaderCache.close();
    //     m_ShaderCache.open(GetTempFileName(), std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
    // }

    ///// <summary>
    ///// Get hash of shader
    ///// </summary>
    //[[nodiscard]] SHA256::Bytes GetShaderHash(
    //    RHI::ShaderStage               Stage,
    //    const RHI::MShaderCompileFlags Flags   = RHI::MShaderCompileFlags_Default,
    //    RHI::ShaderProfile             Profile = RHI::ShaderProfile::SP_6_5,
    //    const RHI::ShaderMacros&       Macros  = {})
    //{
    //    SHA256 Hash;
    //    Hash.Append(Stage);
    //    Hash.Append(Flags.ToUllong());
    //    Hash.Append(Profile);
    //    for (auto& [Key, Value] : Macros.Defines)
    //    {
    //        Hash.Append(Key);
    //        Hash.Append(Value);
    //    }
    //    return Hash.Digest();
    //}

    // void ShaderAsset::PreloadShader(
    //     RHI::ShaderStage               Stage,
    //     const RHI::MShaderCompileFlags Flags,
    //     RHI::ShaderProfile             Profile,
    //     const RHI::ShaderMacros&       Macros)
    //{
    //     auto Hash = GetShaderHash(Stage, Flags, Profile, Macros);
    // }

    ////

    ///**
    // * Shader cache file format:
    // *
    // * Hash (32 bytes)
    // * Shader size (8 bytes)
    // * Shader data (Shader size bytes)
    // *
    // * Repeat for each shader
    // */
    // Ptr<RHI::IShader> ShaderAsset::LoadShader(
    //    RHI::ShaderStage               Stage,
    //    const RHI::MShaderCompileFlags Flags,
    //    RHI::ShaderProfile             Profile,
    //    const RHI::ShaderMacros&       Macros)
    //{
    //}

    // std::filesystem::path ShaderAsset::GetTempFileName() const
    //{
    //     return std::filesystem::path(
    //         StringUtils::Format(
    //             "{}_{}.nsc",
    //             std::filesystem::temp_directory_path().string(),
    //             m_ShaderCacheName));
    // }

    // bool ShaderAsset::SeekShader(
    //     const SHA256::Bytes&        Hash,
    //     std::unique_ptr<uint8_t[]>* ShaderData,
    //     size_t*                     ShaderSize)
    //{
    //     m_ShaderCache.seekg(0, std::ios::beg);
    //     for (size_t i = 0; i < m_FileSize; i = m_ShaderCache.tellg())
    //     {
    //         SHA256::Bytes CurHash;
    //         m_ShaderCache.read(std::bit_cast<char*>(CurHash.data()), CurHash.size());

    //        if (Hash == Hash)
    //        {
    //            if (ShaderSize)
    //            {
    //                m_ShaderCache.read(std::bit_cast<char*>(*ShaderSize), sizeof(*ShaderSize));

    //                *ShaderData = std::make_unique<uint8_t[]>(*ShaderSize);
    //                m_ShaderCache.read(std::bit_cast<char*>(ShaderData->get()), *ShaderSize);

    //                return true;
    //            }
    //        }
    //        else
    //        {
    //            uint64_t ShaderSize = 0;
    //            m_ShaderCache.read(std::bit_cast<char*>(&ShaderSize), sizeof(ShaderSize));

    //            m_ShaderCache.seekg(ShaderSize, std::ios::cur);
    //        }
    //    }
    //    return false;
    //}

    // Ptr<RHI::IShader> ShaderAsset::LoadShaderImpl(
    //     bool                           CreateIfExists,
    //     RHI::ShaderStage               Stage,
    //     const RHI::MShaderCompileFlags Flags,
    //     RHI::ShaderProfile             Profile,
    //     const RHI::ShaderMacros&       Macros)
    //{
    //     NEON_ASSERT(!m_ShaderCode.empty(), "Shader code is empty");
    //     NEON_ASSERT(m_ShaderCache.is_open(), "Shader cache file is not open");

    //    // Check if shader is already compiled
    //    auto Hash = GetShaderHash(Stage, Flags, Profile, Macros);

    //    std::unique_ptr<uint8_t[]> ShaderData;
    //    size_t                     ShaderSize = 0;
    //    if (SeekShader(Hash, &ShaderData, &ShaderSize))
    //    {
    //        if (CreateIfExists)
    //        {
    //        }
    //        return RHI::IShader::Create(std::move(ShaderData), ShaderSize);
    //    }

    //    RHI::ShaderCompileDesc Desc{
    //        .Macros     = Macros,
    //        .SourceCode = m_ShaderCode,
    //        .Profile    = Profile,
    //        .Stage      = Stage,
    //        .Flags      = Flags,
    //    };
    //    auto Shader = RHI::IShader::Create(Desc);

    //    // Save shader to cache
    //    m_ShaderCache.seekp(0, std::ios::end);
    //    m_ShaderCache.write(std::bit_cast<const char*>(Hash.data()), Hash.size());

    //    auto CodeByte = Shader->GetByteCode();

    //    ShaderSize = CodeByte.Size;
    //    m_ShaderCache.write(std::bit_cast<const char*>(&ShaderSize), sizeof(ShaderSize));
    //    m_ShaderCache.write(std::bit_cast<const char*>(CodeByte.Data), CodeByte.Size);

    //    m_FileSize = m_ShaderCache.tellp();

    //    return Shader;
    //}

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