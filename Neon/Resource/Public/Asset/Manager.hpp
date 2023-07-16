#pragma once

#include <Core/Neon.hpp>
#include <Core/String.hpp>
#include <Asset/Handle.hpp>
#include <Asio/Coroutines.hpp>
#include <future>
#include <vector>

namespace Neon::AAsset
{
    class IPackage;
    class IAssetHandler;
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
        [[nodiscard]] Asio::CoGenerator<IPackage*> GetPackages() const noexcept;

    public:
        /// <summary>
        /// Register an asset handler.
        /// </summary>
        void RegisterHandler(
            const StringU8&     Name,
            UPtr<IAssetHandler> Handler);

        /// <summary>
        /// Register an asset handler.
        /// </summary>
        template<typename _Ty>
            requires std::is_base_of_v<IAssetHandler, _Ty>
        void RegisterHandler(
            const StringU8& Name)
        {
            RegisterHandler(Name, UPtr<IAssetHandler>{ std::make_unique<_Ty>() });
        }

        /// <summary>
        /// Unregister an asset handler.
        /// </summary>
        void UnregisterHandler(
            const StringU8& Name);

    public:
        /// <summary>
        /// Load an asset from a handle.
        /// </summary>
        std::future<Ref<IAsset>> Load(
            const Handle& ResHandle);

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
        std::vector<UPtr<IPackage>> m_Packages;
        UPtr<Storage>               m_Storage;
    };
} // namespace Neon::AAsset