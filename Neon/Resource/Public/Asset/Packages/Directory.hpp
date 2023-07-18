#pragma once

#include <Core/String.hpp>
#include <Asset/Package.hpp>

#include <filesystem>

namespace Neon::AAsset
{
    class PackageDirectory : public IPackage
    {
        struct HandleInfo
        {
            StringU8   FilePath;
            StringU8   HandlerName;
            std::mutex Mutex;
        };
        using HandleToFilePathMap = std::unordered_map<
            Handle, HandleInfo>;

    public:
        PackageDirectory(
            const std::filesystem::path& DirPath);

        void AddAsset(
            const AssetAddDesc& Desc,
            const Ptr<IAsset>&  Asset) override;

        void RemoveAsset(
            const Handle& AssetHandle) override;

    protected:
        Ptr<IAsset> Load(
            Storage*      AssetStorage,
            const Handle& ResHandle) override;

        void Flush(
            Storage* AssetStorage) override;

        std::vector<Handle> GetAssets() const override;

    private:
        /// <summary>
        /// Loading assets without checking for dependencies.
        /// </summary>
        Ptr<IAsset> LoadNoDep(
            AssetDependencyGraph& Graphg,
            Storage*              AssetStorage,
            const Handle&         ResHandle);

    private:
        HandleToFilePathMap m_HandleToFilePathMap;
    };
} // namespace Neon::AAsset