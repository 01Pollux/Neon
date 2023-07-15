#pragma once

#include <Core/String.hpp>
#include <Asset/Package.hpp>
#include <Asset/Handle.hpp>

#include <cppcoro/generator.hpp>
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
            Storage* Storage) override;

        void Unmount(
            Storage* Storage) override;

        [[nodiscard]] cppcoro::task<Ptr<IAsset>> Load(
            Storage*      AssetStorage,
            const Handle& ResHandle) override;

    private:
        /// <summary>
        /// Enumerate all dependencies of the given handle.
        /// </summary>
        [[nodiscard]] cppcoro::generator<std::vector<Handle>> EnumerateDependencies(
            const Handle& ResHandle);

    private:
        HandleToFilePathMap m_HandleToFilePathMap;
    };
} // namespace Neon::AAsset