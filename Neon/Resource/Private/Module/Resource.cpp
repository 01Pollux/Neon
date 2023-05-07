#include <ResourcePCH.hpp>
#include <Module/Resource.hpp>
#include <future>

namespace Neon::Module
{
    ResourceManager::ResourceManager(
        Neon::World&             World,
        Asset::IResourceManager* Manager)
    {
        m_Manager.reset(Manager);
        World->system("Resource::Flush")
            .kind(flecs::OnLoad)
            .iter([this](flecs::iter& Iter)
                  { FlushPacks(Iter); });
    }

    Asset::IResourceManager* ResourceManager::Get()
    {
        return m_Manager.get();
    }

    void ResourceManager::FlushPacks(
        flecs::iter& Iter)
    {
        std::vector<std::future<void>> AsyncFlush;
        // Flush each pack in seperate threads
        for (auto& Pack : m_Manager->GetPacks() | std::views::values)
        {
            AsyncFlush.emplace_back(
                std::async([&Pack]
                           { Pack->Flush(); }));
        }
    }
} // namespace Neon::Module