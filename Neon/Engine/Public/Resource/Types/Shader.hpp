#pragma once

#include <Core/SHA256.hpp>

#include <Resource/Asset.hpp>
#include <Resource/Handler.hpp>
#include <RHI/Shader.hpp>

#include <map>
#include <fstream>
#include <future>

namespace Neon::Asset
{
    class ShaderLibraryAsset;

    using ShaderModuleId = uint32_t;

    /// <summary>
    /// Shader module (Contains all shader stages)
    /// Shader stages are loaded or compiled by the specified parameters
    /// They are stored in a map with the hash of the parameters as key and the shader as value
    /// </summary>
    class ShaderModule
    {
        friend class ShaderLibraryAsset;

        struct PrivateByteCode
        {
            std::unique_ptr<uint8_t[]> Data;
            size_t                     Size;
        };

    public:
        ShaderModule(
            const StringU8&         ModName,
            Ptr<ShaderLibraryAsset> Library,
            ShaderModuleId          Id);

        /// <summary>
        /// Load or compile shader stage by the specified parameters
        /// </summary>
        RHI::IShader* LoadStage(
            RHI::ShaderStage               Stage,
            const RHI::MShaderCompileFlags Flags   = RHI::MShaderCompileFlags_Default,
            RHI::ShaderProfile             Profile = RHI::ShaderProfile::SP_6_5,
            const RHI::ShaderMacros&       Macros  = {});

    private:
        /// <summary>
        /// Seek shader in cache
        /// </summary>
        bool SeekShader(
            const SHA256::Bytes&        Hash,
            std::unique_ptr<uint8_t[]>* ShaderData,
            size_t*                     ShaderSize);

        /// <summary>
        /// Write shader to cache
        /// </summary>
        void WriteCache(
            const SHA256::Bytes& Hash,
            RHI::IShader*        Shader);

        /// <summary>
        /// Remove all loaded binaries from this shader module
        /// </summary>
        void Optimize();

    private:
        Ptr<ShaderLibraryAsset> m_Library;
        ShaderModuleId          m_Id;

        std::map<SHA256::Bytes, UPtr<RHI::IShader>> m_Binaries;

        size_t       m_FileSize = 0;
        std::fstream m_ShaderCache;

        std::mutex m_ModuleAccessMutex;
    };

    /// <summary>
    /// Shader library asset (Contains all shader modules)
    /// Shader modules are stored in a single file and loaded on demand
    /// They can be added, removed and optimized at runtime
    /// </summary>
    class ShaderLibraryAsset : public IAssetResource
    {
        friend class ShaderModule;

        struct ShaderModuleTable
        {
            Ptr<ShaderModule> Module;
            StringU8          ModName;
            ShaderModuleId    ModId;

            ShaderModuleTable(
                ShaderModuleId          Id,
                StringU8                ModName,
                Ptr<ShaderLibraryAsset> Library) :
                Module(std::make_shared<ShaderModule>(ModName, std::move(Library), Id)),
                ModName(std::move(ModName)),
                ModId(Id)
            {
            }
        };

    public:
        /// <summary>
        /// Get shader module by id
        /// </summary>
        [[nodiscard]] const Ptr<ShaderModule>& LoadModule(
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
        void RemoveModule(
            ShaderModuleId Id);

        /// <summary>
        /// Optimize shader library (clear all loaded binaries)
        /// </summary>
        void Optimize();

    private:
        /// <summary>
        /// Get shader module's code by id
        /// </summary>
        [[nodiscard]] const StringU8* GetModuleCode(
            ShaderModuleId Id);

        /// <summary>
        /// Decompress shader module once
        /// </summary>
        const StringU8& DecompressOnce(
            ShaderModuleId ModOffset);

        /// <summary>
        /// Set shader module by id
        /// </summary>
        [[nodiscard]] void SetModule(
            ShaderModuleId Id,
            StringU8       ModName,
            StringU8       ModCode,
            bool           Compressed);

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
        struct ModuleData
        {
            StringU8 Code;
            bool     Compressed;
        };

        std::map<ShaderModuleId, ModuleData>        m_ModulesData;
        std::map<ShaderModuleId, ShaderModuleTable> m_Modules;

        std::mutex m_LibraryAccessMutex;
    };
} // namespace Neon::Asset