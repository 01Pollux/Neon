#pragma once

#include <Core/Neon.hpp>
#include <Core/String.hpp>

#include <boost/pool/poolfwd.hpp>
#include <Resource/Asset.hpp>
#include <set>
#include <type_traits>

namespace Neon::Asset
{
    class AssetResourceHandlers;
    class DeferredResourceOperator;

    class IAssetPack
    {
        friend class DeferredResourceOperator;

    public:
        using AssetHandleList    = std::vector<AssetHandle, boost::pool_allocator<AssetHandle>>;
        using AssetDependencyMap = std::map<AssetHandle, std::set<AssetHandle>>;

        enum class ContainType : uint8_t
        {
            Missing,
            Loaded,
            NotLoaded
        };

        IAssetPack(
            const AssetResourceHandlers& Handlers,
            DeferredResourceOperator&    DefferedOperator);

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
        /// Load asset from pack file.
        /// </summary>
        template<typename _Ty>
            requires std::derived_from<_Ty, IAssetResource>
        [[nodiscard]] Ptr<_Ty> Load(
            const AssetHandle& Handle)
        {
            auto Resource = Load(Handle);
            return std::dynamic_pointer_cast<_Ty>(Resource.lock());
        }

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
            const AssetHandle& Handle) const = 0;

        /// <summary>
        /// Check if the asset was not loaded, loaded or missing.
        /// </summary>
        virtual AssetHandleList GetAssets() const = 0;

    public:
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

    public:
        /// <summary>
        /// Add dependency to the asset.
        /// </summary>
        void AddDependency(
            const AssetHandle& Resource,
            const AssetHandle& DependsOn);

        /// <summary>
        /// Remove dependency from the asset.
        /// </summary>
        void RemoveDependency(
            const AssetHandle& Resource,
            const AssetHandle& DependsOn);

    protected:
        template<std::invocable<const AssetHandle&> _FTy>
        void IterateDepencies(
            const AssetHandle& Resource,
            _FTy&&             Func)
        {
            if (auto Iter = m_Dependencies.find(Resource); Iter != m_Dependencies.end())
            {
                for (const auto& Dependency : Iter->second)
                {
                    Func(Dependency);
                }
            }
        }

    protected:
        const AssetResourceHandlers& m_Handlers;
        mutable std::mutex           m_PackMutex;
        DeferredResourceOperator&    m_DefferedOperator;
        AssetDependencyMap           m_Dependencies;
    };
} // namespace Neon::Asset