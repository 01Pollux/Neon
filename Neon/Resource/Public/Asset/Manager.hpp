#pragma once

#include <Core/Neon.hpp>
#include <Core/String.hpp>
#include <Asset/Handle.hpp>
#include <cppcoro/generator.hpp>
#include <vector>

namespace Neon::AAsset
{
    class IPackage;
    class IAsset;
    class Storage;

    //

    class Manager
    {
    public:
        Manager();
        NEON_CLASS_NO_COPY(Manager);
        NEON_CLASS_MOVE_DECL(Manager);
        ~Manager();

        /// <summary>
        /// Mount a package from a path.
        /// </summary>
        IPackage* Mount(
            UPtr<IPackage> Package);

        /// <summary>
        /// Unmount a package from a handle.
        /// </summary>
        void Unmount(
            IPackage* Package);

        /// <summary>
        /// Get a package from a handle.
        /// </summary>
        [[nodiscard]] cppcoro::generator<IPackage*> GetPackages() const noexcept;

        //

        /// <summary>
        /// Preload an asset from a handle.
        /// </summary>
        void LoadAsync(
            const Handle& Handle);

        /// <summary>
        /// Load an asset from a handle.
        /// </summary>
        [[nodiscard]] Ref<IAsset> Load(
            const Handle& Handle);

        /// <summary>
        /// Unload an asset from a handle.
        /// </summary>
        void Unload(
            const Handle& Handle);

    private:
        std::vector<UPtr<IPackage>> m_Packages;
        UPtr<Storage>               m_Storage;
    };
} // namespace Neon::AAsset