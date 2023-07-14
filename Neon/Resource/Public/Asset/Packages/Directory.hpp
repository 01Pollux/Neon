#pragma once

#include <Core/String.hpp>
#include <Asset/Package.hpp>
#include <Asset/Handle.hpp>
#include <filesystem>

namespace Neon::AAsset
{
    class PackageDirectory : public IPackage
    {
        using HandleToFilePathMap = std::unordered_map<Handle, StringU8>;

    public:
        PackageDirectory(
            const std::filesystem::path& DirPath);

        void Mount(
            Storage* Storage) override;

        void Unmount(
            Storage* Storage) override;

        [[nodiscard]] Ptr<IAsset> Load(
            const Handle& Handle) override;

    private:
        HandleToFilePathMap m_HandleToFilePathMap;
    };
} // namespace Neon::AAsset