#pragma once

#include <Core/Neon.hpp>
#include <Core/String.hpp>
#include <Resource/Handle.hpp>

#include <map>
#include <mutex>
#include <condition_variable>

#include <queue>
#include <boost/variant.hpp>
#include <boost/pool/pool_alloc.hpp>

namespace Neon::Asset
{
    class IAssetPack;
    class IAssetResource;

    class PendingResourceOperator
    {
    public:
        PendingResourceOperator();

        /// <summary>
        /// Aquire lock on resource thread.
        /// Must be called before any operation on resource.
        /// if the caller is not the resource thread, it will wait until the resource thread is done with its current operation.
        /// </summary>
        template<typename _Mtx>
        std::unique_lock<_Mtx> Lock(
            const IAssetPack* Pack,
            _Mtx&             Mutex) const
        {
            std::unique_lock ResourceLock(Mutex, std::defer_lock);
            if (std::this_thread::get_id() != m_ResourceThread.get_id())
            {
                std::unique_lock WaiterLock(m_WaiterMutex);
                m_WaiterCondition.wait(
                    WaiterLock,
                    [this, Pack]
                    {
                        std::scoped_lock LoaderLock(m_LoaderMutex);
                        return !m_PendingPacksOperations.contains(const_cast<IAssetPack*>(Pack));
                    });

                ResourceLock.lock();
            }
            return ResourceLock;
        }

        /// <summary>
        /// Import asset pack file and overwrite current content of the pack asynchronously.
        /// </summary>
        void ImportAsync(
            IAssetPack*     Pack,
            const StringU8& FilePath);

        /// <summary>
        /// Export asset pack file asynchronously.
        /// </summary>
        void ExportAsync(
            IAssetPack*     Pack,
            const StringU8& FilePath);

        /// <summary>
        /// Load asset from pack file asynchronously.
        /// </summary>
        void LoadAsync(
            IAssetPack*        Pack,
            const AssetHandle& Handle);

        /// <summary>
        /// Save asset to pack file asynchronously.
        /// </summary>
        void SaveAsync(
            IAssetPack*                Pack,
            const AssetHandle&         Handle,
            const Ptr<IAssetResource>& Resource);

    private:
        struct ImportOperation
        {
            StringU8 Path;
        };

        struct ExportOperation
        {
            StringU8 Path;
        };

        struct LoadOperation
        {
            AssetHandle Handle;
        };

        struct SaveOperation
        {
            AssetHandle         Handle;
            Ptr<IAssetResource> Resource;
        };

        using PendingOperations      = boost::variant<ImportOperation, ExportOperation, LoadOperation, SaveOperation>;
        using PendingAssetOperations = std::vector<PendingOperations, boost::pool_allocator<PendingOperations>>;
        using PendingPacksOperations = std::map<IAssetPack*, PendingAssetOperations>;

    private:
        PendingPacksOperations m_PendingPacksOperations;

        mutable std::mutex                  m_LoaderMutex, m_WaiterMutex;
        mutable std::condition_variable_any m_LoaderCondition, m_WaiterCondition;
        std::jthread                        m_ResourceThread;
    };
} // namespace Neon::Asset