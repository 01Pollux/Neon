#include "ResourcePCH.hpp"
#include "Pack.hpp"

namespace Neon::Asset
{
    IAssetPack::IAssetPack(
        const AssetResourceHandlers& Handlers) :
        m_Handlers(Handlers)
    {
    }

    void IAssetPack::Flush()
    {
        std::lock_guard Lock(m_AsyncMutex);

        if (!m_PendingOperations.empty())
        {
            VariantVisitor Visitor{
                [this](const ImportOperation& Op)
                {
                    Import(Op.Path);
                },
                [this](const ExportOperation& Op)
                {
                    Export(Op.Path);
                },
                [this](const LoadOperation& Op)
                {
                    Load(Op.Handle);
                },
                [this](const SaveOperation& Op)
                {
                    Save(Op.Handle, Op.Resource);
                },
            };
            for (auto& Op : m_PendingOperations)
            {
                boost::apply_visitor(
                    Visitor,
                    Op);
            }
            m_PendingOperations.clear();
        }
    }

    void IAssetPack::ImportAsync(
        const StringU8& FilePath)
    {
        std::lock_guard Lock(m_AsyncMutex);
        m_PendingOperations.emplace_back(ImportOperation{ FilePath });
    }

    void IAssetPack::ExportAsync(
        const StringU8& FilePath)
    {
        std::lock_guard Lock(m_AsyncMutex);
        m_PendingOperations.emplace_back(ExportOperation{ FilePath });
    }

    void IAssetPack::LoadAsync(
        const AssetHandle& Handle)
    {
        std::lock_guard Lock(m_AsyncMutex);
        m_PendingOperations.emplace_back(LoadOperation{ Handle });
    }

    void IAssetPack::SaveAsync(
        const AssetHandle&         Handle,
        const Ptr<IAssetResource>& Resource)
    {
        std::lock_guard Lock(m_AsyncMutex);
        m_PendingOperations.emplace_back(SaveOperation{ Handle, Resource });
    }
} // namespace Neon::Asset