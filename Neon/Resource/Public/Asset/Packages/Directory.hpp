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
            StringU8 FilePath;
            StringU8 HandlerName;
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
        Asio::CoLazy<Ptr<IAsset>> Load(
            Storage*      AssetStorage,
            const Handle& ResHandle) override;

        void Flush() override;

    private:
        HandleToFilePathMap m_HandleToFilePathMap;
    };
} // namespace Neon::AAsset