#pragma once

#include <Asset/Asset.hpp>
#include <Asset/Metadata.hpp>
#include <IO/Archive2.hpp>

#include <boost/serialization/vector.hpp>
#include <unordered_set>
#include <fstream>
#include <queue>

namespace Neon::AAsset
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
            std::ifstream&       Stream,
            const Handle&        AssetGuid,
            StringU8             Path,
            const AssetMetaData& LoaderData) = 0;

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
        /// Link an asset for depdendency reading.
        /// </summary>
        void Link(
            const Handle&      AssetGuid,
            const Ptr<IAsset>& Asset)
        {
            m_Assets.emplace(AssetGuid, Asset);
        }

        /// <summary>
        /// Read the dependencies of an asset.
        /// </summary>
        [[nodiscard]] Ptr<IAsset> ReadOne(
            const Handle& AssetGuid) const
        {
            auto Iter = m_Assets.find(AssetGuid);
            return Iter != m_Assets.end() ? Iter->second : nullptr;
        }

        /// <summary>
        /// Read the dependencies of an asset.
        /// </summary>
        template<typename _Archive>
            requires requires(_Archive& Archive, Handle& Guid) { Archive >> Guid; }
        [[nodiscard]] auto ReadOne(
            _Archive& Archive) const
        {
            Handle AssetGuid;
            Archive >> AssetGuid;
            return ReadOne(AssetGuid);
        }

        /// <summary>
        /// Read the dependencies of an asset.
        /// </summary>
        [[nodiscard]] std::vector<Ptr<IAsset>> ReadMany(
            std::span<const Handle> AssetGuid) const
        {
            std::vector<Ptr<IAsset>> Assets;
            Assets.reserve(AssetGuid.size());
            for (const auto& Guid : AssetGuid)
            {
                Assets.push_back(ReadOne(Guid));
            }
            return Assets;
        }

        /// <summary>
        /// Read the dependencies of an asset.
        /// </summary>
        template<typename _Archive>
            requires requires(_Archive& Archive, std::vector<Handle>& Guids) { Archive >> Guids; }
        [[nodiscard]] auto ReadMany(
            _Archive& Archive) const
        {
            std::vector<Handle> Guids;
            Archive >> Guids;
            return ReadMany(Guids);
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
} // namespace Neon::AAsset