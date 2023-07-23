#pragma once

#include <Asset/Asset.hpp>
#include <Asset/Metadata.hpp>
#include <IO/Archive2.hpp>

#include <boost/serialization/vector.hpp>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <queue>

namespace Neon::Asset
{
    class DependencyReader;
    class DependencyWriter;

    class IAssetHandler
    {
    public:
        /// <summary>
        /// Query if this asset handler can handle the given asset.
        /// </summary>
        virtual bool CanHandle(
            const Ptr<IAsset>& Asset) = 0;

        /// <summary>
        /// Load the asset from an input stream.
        /// </summary>
        virtual Ptr<IAsset> Load(
            std::ifstream&                  Stream,
            const Asset::DependencyReader& DepReader,
            const Handle&                   AssetGuid,
            StringU8                        Path,
            const AssetMetaData&            LoaderData) = 0;

        /// <summary>
        /// Save the asset to an output stream.
        /// </summary>
        virtual void Save(
            std::fstream&      Stream,
            DependencyWriter&  DepWriter,
            const Ptr<IAsset>& Asset,
            AssetMetaData&     LoaderData) = 0;
    };

    //

    class DependencyReader
    {
    public:
        /// <summary>
        /// Read the dependencies of an asset.
        /// </summary>
        template<typename _Ty, typename _Archive>
            requires std::is_base_of_v<IAsset, _Ty>
        [[nodiscard]] Ptr<_Ty> ReadOne(
            _Archive& Archive) const
        {
            Handle AssetGuid;
            Archive >> AssetGuid;
            if (AssetGuid != Handle::Null)
            {
                return std::dynamic_pointer_cast<_Ty>(m_Assets[AssetGuid]);
            }
            else
            {
                return nullptr;
            }
        }

        /// <summary>
        /// Read the dependencies of an asset.
        /// </summary>
        template<typename _Ty, typename _Archive>
            requires std::is_base_of_v<IAsset, _Ty>
        [[nodiscard]] auto ReadMany(
            _Archive& Archive) const
        {
            std::vector<Handle> ChildGuids;
            Archive >> ChildGuids;

            std::vector<Ptr<_Ty>> ChildAssets;
            ChildAssets.reserve(ChildGuids.size());

            for (const auto& ChildGuid : ChildGuids)
            {
                Ptr<_Ty> Asset;
                if (ChildGuid != Handle::Null)
                {
                    auto AssetIter = m_Assets.find(ChildGuid);
                    if (AssetIter != m_Assets.end())
                    {
                        Asset = std::dynamic_pointer_cast<_Ty>(AssetIter->second);
                    }
                }
                ChildAssets.emplace_back(std::move(Asset));
            }

            return ChildAssets;
        }

    public:
        /// <summary>
        /// Internal use only.
        /// Link an asset for depdendency reading.
        /// </summary>
        void Link(
            const Handle&      AssetGuid,
            const Ptr<IAsset>& Asset)
        {
            m_Assets.emplace(AssetGuid, Asset);
        }

    private:
        std::unordered_map<Handle, Ptr<IAsset>> m_Assets;
    };

    //

    class DependencyWriter
    {
    public:
        /// <summary>
        /// Read the dependencies of an asset.
        /// </summary>
        template<typename _Archive, typename _Ty>
            requires std::is_base_of_v<IAsset, _Ty>
        [[nodiscard]] void WriteOne(
            _Archive&       Archive,
            const Ptr<_Ty>& Asset)
        {
            if (Asset)
            {
                Archive << Asset->GetGuid();
                m_Assets.emplace(Asset);
            }
            else
            {
                Archive << Handle::Null;
            }
        }

        /// <summary>
        /// Read the dependencies of an asset.
        /// </summary>
        template<typename _Archive, typename _Ty>
        [[nodiscard]] void WriteMany(
            _Archive&  Archive,
            const _Ty& Assets)
        {
            auto ChildGuids =
                Assets |
                std::views::transform(
                    [this](const auto& Child)
                    {
                        if (Child)
                        {
                            m_Assets.emplace(Child);
                            return Child->GetGuid();
                        }
                        return Handle::Null;
                    }) |
                std::ranges::to<std::vector<Handle>>();

            Archive << ChildGuids;
        }

    public:
        /// <summary>
        /// Internal use only.
        /// Get the dependencies of an asset.
        /// </summary>
        auto& GetDependencies() noexcept
        {
            return m_Assets;
        }

    private:
        std::unordered_set<Ptr<IAsset>> m_Assets;
    };
} // namespace Neon::Asset