#include <ResourcePCH.hpp>
#include <Resource/Pack.hpp>
#include <Resource/Operator.hpp>

namespace Neon::Asset
{
    IAssetPack::IAssetPack(
        const AssetResourceHandlers& Handlers,
        PendingResourceOperator&     PendingOperator) :
        m_Handlers(Handlers),
        m_PendingOperator(PendingOperator)
    {
    }

    void IAssetPack::ImportAsync(
        const StringU8& FilePath)
    {
        m_PendingOperator.ImportAsync(this, FilePath);
    }

    void IAssetPack::ExportAsync(
        const StringU8& FilePath)
    {
        m_PendingOperator.ExportAsync(this, FilePath);
    }

    void IAssetPack::LoadAsync(
        const AssetHandle& Handle)
    {
        m_PendingOperator.LoadAsync(this, Handle);
    }

    void IAssetPack::SaveAsync(
        const AssetHandle&         Handle,
        const Ptr<IAssetResource>& Resource)
    {
        m_PendingOperator.SaveAsync(this, Handle, Resource);
    }
} // namespace Neon::Asset