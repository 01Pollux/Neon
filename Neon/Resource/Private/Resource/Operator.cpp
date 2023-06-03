#include <ResourcePCH.hpp>
#include <Resource/Operator.hpp>
#include <Resource/Pack.hpp>

#include <Log/Logger.hpp>

namespace Neon::Asset
{
    DeferredResourceOperator::DeferredResourceOperator()
    {
        m_ResourceThread = std::jthread(
            [this](std::stop_token StopToken)
            {
                while (true)
                {
                    std::unique_lock Lock(m_LoaderMutex);
                    if (!m_LoaderCondition.wait(Lock, StopToken, [this]
                                                { return !m_PendingPacksOperations.empty(); }))
                    {
                        return;
                    }

                    for (auto& [Pack, Operations] : m_PendingPacksOperations)
                    {
                        try
                        {
                            VariantVisitor Visitor{
                                [Pack](const ImportOperation& Op)
                                {
                                    Pack->Import(Op.Path);
                                },
                                [Pack](const ExportOperation& Op)
                                {
                                    Pack->Export(Op.Path);
                                },
                                [Pack](const LoadOperation& Op)
                                {
                                    Pack->Load(Op.Handle);
                                },
                                [Pack](const SaveOperation& Op)
                                {
                                    Pack->Save(Op.Handle, Op.Resource);
                                },
                            };
                            for (auto& Op : Operations)
                            {
                                boost::apply_visitor(
                                    Visitor,
                                    Op);
                            }
                        }
                        catch (const std::exception& Exception)
                        {
                            NEON_ERROR_TAG("Resource", "Exception in resource thread: {}", Exception.what());
                        }
                    }
                    m_PendingPacksOperations.clear();
                    m_WaiterCondition.notify_all();
                }
            });
    }

    void DeferredResourceOperator::ImportAsync(
        IAssetPack*     Pack,
        const StringU8& FilePath)
    {
        {
            std::scoped_lock LoaderLock(m_LoaderMutex);
            m_PendingPacksOperations[Pack].emplace_back(ImportOperation{ FilePath });
        }
        m_LoaderCondition.notify_one();
    }

    void DeferredResourceOperator::ExportAsync(
        IAssetPack*     Pack,
        const StringU8& FilePath)
    {
        {
            std::scoped_lock LoaderLock(m_LoaderMutex);
            m_PendingPacksOperations[Pack].emplace_back(ExportOperation{ FilePath });
        }
        m_LoaderCondition.notify_one();
    }

    void DeferredResourceOperator::LoadAsync(
        IAssetPack*        Pack,
        const AssetHandle& Handle)
    {
        {
            std::scoped_lock LoaderLock(m_LoaderMutex);
            m_PendingPacksOperations[Pack].emplace_back(LoadOperation{ Handle });
        }
        m_LoaderCondition.notify_one();
    }

    void DeferredResourceOperator::SaveAsync(
        IAssetPack*                Pack,
        const AssetHandle&         Handle,
        const Ptr<IAssetResource>& Resource)
    {
        {
            std::scoped_lock LoaderLock(m_LoaderMutex);
            m_PendingPacksOperations[Pack].emplace_back(SaveOperation{ Handle, Resource });
        }
        m_LoaderCondition.notify_one();
    }
} // namespace Neon::Asset