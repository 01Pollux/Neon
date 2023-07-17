#pragma once

#include <Core/Neon.hpp>
#include <Core/String.hpp>
#include <Asset/Handle.hpp>
#include <Asio/Coroutines.hpp>
#include <mutex>
#include <future>
#include <vector>

namespace Neon::AAsset
{
    class IPackage;
    class IAssetHandler;
    class IAsset;
    class Storage;

    //

    // clang-format off
    template<typename _Ty>
    concept CustomAssetHandler = requires() {
            std::is_base_of_v<IAssetHandler, _Ty>;
            { _Ty::HandlerName } -> std::convertible_to<StringU8>;
        };
    // clang-format on

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
        /// Get loaded packages.
        /// </summary>
        [[nodiscard]] Asio::CoGenerator<IPackage*> GetPackages() const noexcept;

        /// <summary>
        /// Get loaded package assets.
        /// </summary>
        [[nodiscard]] Asio::CoGenerator<const AAsset::Handle&> GetPackageAssets(
            IPackage* Package) const noexcept;

    public:
        /// <summary>
        /// Register an asset handler.
        /// Not thread-safe.
        /// </summary>
        void RegisterHandler(
            const StringU8&     Name,
            UPtr<IAssetHandler> Handler);

        /// <summary>
        /// Register an asset handler.
        /// Not thread-safe.
        /// </summary>
        template<CustomAssetHandler _Ty>
        void RegisterHandler()
        {
            RegisterHandler(_Ty::HandlerName, UPtr<IAssetHandler>{ std::make_unique<_Ty>() });
        }

        /// <summary>
        /// Unregister an asset handler.
        /// Not thread-safe.
        /// </summary>
        void UnregisterHandler(
            const StringU8& Name);

    public:
        /// <summary>
        /// Load an asset from a handle.
        /// </summary>
        std::future<Ref<IAsset>> Load(
            IPackage*     Package,
            const Handle& ResHandle);

        /// <summary>
        /// Flush the package.
        /// </summary>
        std::future<void> Flush(
            IPackage* Package);

        /// <summary>
        /// Unload an asset from a handle.
        /// </summary>
        void Unload(
            const Handle& ResHandle);

    private:
        /// <summary>
        /// Register the standard asset handlers.
        /// </summary>
        void RegisterStandardHandlers();

    private:
        mutable std::mutex          m_PackagesMutex;
        std::vector<UPtr<IPackage>> m_Packages;
        UPtr<Storage>               m_Storage;
    };
} // namespace Neon::AAsset