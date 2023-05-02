#include <ResourcePCH.hpp>
#include <Resource/Manager.hpp>

#include <Private/Resource/ZipPack.hpp>

namespace Neon::Asset
{
    void IResourceManager::Import(
        const StringU8& Path)
    {
        // TODO: Can load multiple packs of different types
        throw std::runtime_error("TODO");
    }

    Ref<IAssetResource> IResourceManager::Load(
        const AssetHandle& Handle)
    {
        throw std::runtime_error("TODO");
        return Ref<IAssetResource>();
    }

    void IResourceManager::Save(
        const AssetHandle&         Handle,
        const Ptr<IAssetResource>& Resource)
    {
        auto& Pack = m_LoadedPacks[Handle.GetPack()];
        if (!Pack)
        {
            Pack = std::make_shared<ZipAssetPack>();
        }
        Pack->Save(m_Handlers, Handle, Resource);
    }
} // namespace Neon::Asset