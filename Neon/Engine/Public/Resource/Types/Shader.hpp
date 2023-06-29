#pragma once

#include <Resource/Asset.hpp>
#include <Resource/Handler.hpp>
#include <Asio/ThreadPool.hpp>
#include <RHI/Shader.hpp>

#include <fstream>
#include <future>

namespace Neon::Asset
{
    class ShaderModuleId
    {
        friend class ShaderLibraryAsset;

    public:
        ShaderModuleId(uint32_t Id) :
            m_Id(Id)
        {
        }

    private:
        uint32_t m_Id;
    };

    /// <summary>
    /// Shader module (Contains all shader stages)
    /// Shader stages are loaded or compiled by the specified parameters
    /// They are stored in a map with the hash of the parameters as key and the shader as value
    /// </summary>
    class ShaderModule
    {
        friend class ShaderLibraryAsset;

        struct ByteCode
        {
            std::unique_ptr<uint8_t[]> Data;
            size_t                     Size;
        };

    public:
        /// <summary>
        /// Load or compile shader stage by the specified parameters
        /// </summary>
        std::future<RHI::IShader*> LoadStage(
            RHI::ShaderStage               Stage,
            const RHI::MShaderCompileFlags Flags   = RHI::MShaderCompileFlags_Default,
            RHI::ShaderProfile             Profile = RHI::ShaderProfile::SP_6_5,
            const RHI::ShaderMacros&       Macros  = {});

    protected:
        ShaderModule(ShaderLibraryAsset* Library) :
            m_Library(Library)
        {
        }

    private:
        ShaderLibraryAsset*                         m_Library;
        std::map<SHA256::Bytes, UPtr<RHI::IShader>> m_Binaries;
    };

    /// <summary>
    /// Shader library asset (Contains all shader modules)
    /// Shader modules are stored in a single file and loaded on demand
    /// They can be added, removed and optimized at runtime
    /// </summary>
    class ShaderLibraryAsset : public IAssetResource
    {
        struct ShaderModuleTable
        {
            ShaderModuleId ModId;
            StringU8       ModName;
            size_t         ModOffset;
            size_t         ModSize;
        };

    public:
        /// <summary>
        /// Get shader module by id
        /// </summary>
        [[nodiscard]] ShaderModule* LoadModule(
            ShaderModuleId Id);

        /// <summary>
        /// Set shader module by id
        /// </summary>
        [[nodiscard]] void SetModule(
            ShaderModuleId Id,
            StringU8       ModName,
            StringU8       ModCode);

        /// <summary>
        /// Remove shader module by id
        /// </summary>
        /// <param name="Id"></param>
        void RemoveModule(
            ShaderModuleId Id)
        {
            SetModule(Id, {}, {});
        }

        /// <summary>
        /// Optimize shader library (Remove unused loaded binaries)
        /// </summary>
        void Optimize();

    private:
        std::vector<ShaderModuleTable> m_Modules;
        std::vector<uint8_t>           m_ModulesData;
        Asio::ThreadPool<>             m_ThreadPool;
    };

    class ShaderAsset : public IAssetResource
    {
    public:
        ShaderAsset(
            const StringU8& CacheName,
            const StringU8& Code = {});

        /// <summary>
        /// Get shader code
        /// </summary>
        const StringU8& GetCode() const;

        /// <summary>
        /// Set shader code
        /// </summary>
        void SetCode(
            const StringU8& Code);

        /// <summary>
        /// Preload shader (async)
        /// </summary>
        void PreloadShader(
            RHI::ShaderStage               Stage,
            const RHI::MShaderCompileFlags Flags   = RHI::MShaderCompileFlags_Default,
            RHI::ShaderProfile             Profile = RHI::ShaderProfile::SP_6_5,
            const RHI::ShaderMacros&       Macros  = {});

        /// <summary>
        /// Load shader immediately
        /// </summary>
        [[nodiscard]] Ptr<RHI::IShader> LoadShader(
            RHI::ShaderStage               Stage,
            const RHI::MShaderCompileFlags Flags   = RHI::MShaderCompileFlags_Default,
            RHI::ShaderProfile             Profile = RHI::ShaderProfile::SP_6_5,
            const RHI::ShaderMacros&       Macros  = {});

    private:
        /// <summary>
        /// Get temp file name
        /// </summary>
        [[nodiscard]] std::filesystem::path GetTempFileName() const;

        /// <summary>
        /// Seek shader in cache
        /// </summary>
        bool SeekShader(
            const SHA256::Bytes&        Hash,
            std::unique_ptr<uint8_t[]>* ShaderData = nullptr,
            size_t*                     ShaderSize = nullptr);

        /// <summary>
        /// Load shader immediately
        /// </summary>
        [[nodiscard]] Ptr<RHI::IShader> LoadShaderImpl(
            bool                           CreateIfExists,
            RHI::ShaderStage               Stage,
            const RHI::MShaderCompileFlags Flags   = RHI::MShaderCompileFlags_Default,
            RHI::ShaderProfile             Profile = RHI::ShaderProfile::SP_6_5,
            const RHI::ShaderMacros&       Macros  = {});

    public:
        class Handler : public IAssetResourceHandler
        {
        public:
            bool CanCastTo(
                const Ptr<IAssetResource>& Resource) override;

            Ptr<IAssetResource> Load(
                IAssetPack*    Pack,
                IO::InArchive& Archive,
                size_t         DataSize) override;

            void Save(
                IAssetPack*                Pack,
                const Ptr<IAssetResource>& Resource,
                IO::OutArchive&            Archive) override;
        };

    private:
        StringU8     m_ShaderCode;
        StringU8     m_ShaderCacheName;
        std::fstream m_ShaderCache;
        size_t       m_FileSize = 0;

        std::map<SHA256::Bytes, std::future<void()>> m_PreloadingShaders;
    };
} // namespace Neon::Asset