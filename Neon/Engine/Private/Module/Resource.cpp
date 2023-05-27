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

    void ResourceManager::Run()
    {
        FlushPacks();
    }

    void ResourceManager::FlushPacks()
    {
        // Flush each pack in seperate threads
        std::vector<std::future<void>> AsyncFlush;
        for (auto& Pack : m_Manager->GetPacks() | std::views::values)
        {
            AsyncFlush.emplace_back(
                std::async([&Pack]
                           { Pack->Flush(); }));
        }
    }
} // namespace Neon::Module