#include <ResourcePCH.hpp>
#include <Resource/Pack.hpp>
#include <Resource/Operator.hpp>

namespace Neon::Asset
{
    IAssetPack::IAssetPack(
        const AssetResourceHandlers& Handlers,
        DeferredResourceOperator&    DefferedOperator) :
        m_Handlers(Handlers),
        m_DefferedOperator(DefferedOperator)
    {
    }

    void IAssetPack::ImportAsync(
        const StringU8& FilePath)
    {
        m_DefferedOperator.ImportAsync(this, FilePath);
    }

    void IAssetPack::ExportAsync(
        const StringU8& FilePath)
    {
        m_DefferedOperator.ExportAsync(this, FilePath);
    }

    void IAssetPack::LoadAsync(
        const AssetHandle& Handle)
    {
        m_DefferedOperator.LoadAsync(this, Handle);
    }

    void IAssetPack::SaveAsync(
        const AssetHandle&         Handle,
        const Ptr<IAssetResource>& Resource)
    {
        m_DefferedOperator.SaveAsync(this, Handle, Resource);
    }
} // namespace Neon::Asset