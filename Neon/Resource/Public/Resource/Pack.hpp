#pragma once

#include <Core/Neon.hpp>
#include <Core/String.hpp>

#include <Resource/Asset.hpp>
#include <boost/variant.hpp>

#include <queue>
#include <boost/pool/pool_alloc.hpp>

namespace Neon::Asset
{
    class AssetResourceHandlers;
    class IAssetPack
    {
    private:
        struct ImportOperation
        {
            StringU8 Path;
        };

        struct ExportOperation
        {
            StringU8 Path;
        };

        struct LoadOperation
        {
            AssetHandle Handle;
        };

        struct SaveOperation
        {
            AssetHandle         Handle;
            Ptr<IAssetResource> Resource;
        };

        using PendingOperations      = boost::variant<ImportOperation, ExportOperation, LoadOperation, SaveOperation>;
        using PendingAssetOperations = std::vector<PendingOperations, boost::pool_allocator<PendingOperations>>;

    public:
        enum class ContainType : uint8_t
        {
            Missing,
            Loaded,
            NotLoaded
        };

        IAssetPack(
            const AssetResourceHandlers& Handlers);

        virtual ~IAssetPack() = default;

        /// <summary>
        /// Import asset pack file and overwrite current content of the pack.
        /// </summary>
        virtual void Import(
            const StringU8& FilePath) = 0;

        /// <summary>
        /// Export asset pack file.
        /// </summary>
        virtual void Export(
            const StringU8& FilePath) = 0;

        /// <summary>
        /// Load asset from pack file.
        /// </summary>
        virtual Ref<IAssetResource> Load(
            const AssetHandle& Handle) = 0;

        /// <summary>
        /// Save asset to pack file.
        /// </summary>
        virtual void Save(
            const AssetHandle&         Handle,
            const Ptr<IAssetResource>& Resource) = 0;

        /// <summary>
        /// Check if the asset was not loaded, loaded or missing.
        /// </summary>
        virtual ContainType ContainsResource(
            const AssetHandle& Handle) = 0;

    public:
        /// <summary>
        /// Empty the pending asynchronous operations.
        /// </summary>
        void Flush(
            const std::function<void(float Progress)>& FlushCallback = nullptr);

        /// <summary>
        /// Import asset pack file and overwrite current content of the pack asynchronously.
        /// </summary>
        void ImportAsync(
            const StringU8& FilePath);

        /// <summary>
        /// Export asset pack file asynchronously.
        /// </summary>
        void ExportAsync(
            const StringU8& FilePath);

        /// <summary>
        /// Load asset from pack file asynchronously.
        /// </summary>
        void LoadAsync(
            const AssetHandle& Handle);

        /// <summary>
        /// Save asset to pack file asynchronously.
        /// </summary>
        void SaveAsync(
            const AssetHandle&         Handle,
            const Ptr<IAssetResource>& Resource);

    protected:
        const AssetResourceHandlers& m_Handlers;
        std::recursive_mutex         m_AsyncMutex;

    private:
        PendingAssetOperations m_PendingOperations;
    };
} // namespace Neon::Asset