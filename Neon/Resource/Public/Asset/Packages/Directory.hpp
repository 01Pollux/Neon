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

        void Mount(
            Storage* AssetStorage) override;

        void Unmount(
            Storage* AssetStorage) override;

        [[nodiscard]] Asio::CoLazy<Ptr<IAsset>> Load(
            Storage*      AssetStorage,
            const Handle& ResHandle) override;

    private:
        HandleToFilePathMap m_HandleToFilePathMap;
    };
} // namespace Neon::AAsset