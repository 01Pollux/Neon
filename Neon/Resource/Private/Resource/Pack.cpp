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
        std::scoped_lock Lock(m_PackMutex);
        m_Dependencies[Resource].emplace(DependsOn);
    }

    void IAssetPack::RemoveDependency(
        const AssetHandle& Resource,
        const AssetHandle& DependsOn)
    {
        std::scoped_lock Lock(m_PackMutex);

        auto Iter = m_Dependencies.find(Resource);
        if (Iter != m_Dependencies.end())
        {
            Iter->second.erase(DependsOn);
            if (Iter->second.empty())
            {
                m_Dependencies.erase(Iter);
            }
        }
    }
} // namespace Neon::Asset