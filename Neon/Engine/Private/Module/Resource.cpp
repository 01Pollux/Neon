#include <EnginePCH.hpp>
#include <Module/Resource.hpp>
#include <future>

namespace Neon::Module
{
    ResourceManager::ResourceManager(
        Runtime::DefaultGameEngine* Engine,
        Asset::IResourceManager*    Manager)
    {
        m_Manager.reset(Manager);
    }

    Asset::IResourceManager* ResourceManager::Get()
    {
        return m_Manager.get();
    }
} // namespace Neon::Module