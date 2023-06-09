#include <ResourcePCH.hpp>
#include <Resource/Pack.hpp>
#include <Resource/Operator.hpp>

#include <Log/Logger.hpp>

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
        IterateDepencies(
            Handle,
            [this](const AssetHandle& Dependency)
            {
                LoadAsync(Dependency);
            });
        m_DefferedOperator.LoadAsync(this, Handle);
    }

    void IAssetPack::SaveAsync(
        const AssetHandle&         Handle,
        const Ptr<IAssetResource>& Resource)
    {
        m_DefferedOperator.SaveAsync(this, Handle, Resource);
    }

    //

    void IAssetPack::AddDependency(
        const AssetHandle& Resource,
        const AssetHandle& DependsOn)
    {
        NEON_ASSERT(Resource != DependsOn, "Cannot add self dependency");
        m_DefferedOperator.DependencyChangeAsync(this, Resource, DependsOn, true);
    }

    void IAssetPack::RemoveDependency(
        const AssetHandle& Resource,
        const AssetHandle& DependsOn)
    {
        NEON_ASSERT(Resource != DependsOn, "Cannot remove self dependency");
        m_DefferedOperator.DependencyChangeAsync(this, Resource, DependsOn, false);
    }

    void IAssetPack::RemoveAllDependencies(
        const AssetHandle& Resource)
    {
        m_DefferedOperator.DependencyChangeAsync(this, Resource, Resource, false);
    }
} // namespace Neon::Asset