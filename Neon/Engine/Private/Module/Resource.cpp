#include <EnginePCH.hpp>
#include <Module/Resource.hpp>
#include <Resource/Pack.hpp>

#include <Resource/Types/Logger.hpp>

namespace Neon::Module
{
    ResourceManager::ResourceManager(
        Runtime::DefaultGameEngine* Engine,
        Asset::IResourceManager*    Manager) :
        m_Manager(Manager)
    {
        const auto LoggerAssetUid = Asset::AssetHandle::FromString("d0b50bba-f800-4c18-a595-fd5c4b380190");

        //

        auto Pack = Manager->LoadPack("__neon", "neonrt.np");

        // Set global logger settings
        {
            auto Logger = Pack->Load<Asset::LoggerAsset>(LoggerAssetUid);
            Logger->SetGlobal();
        }
    }

    Asset::IResourceManager* ResourceManager::Get()
    {
        return m_Manager.get();
    }
} // namespace Neon::Module