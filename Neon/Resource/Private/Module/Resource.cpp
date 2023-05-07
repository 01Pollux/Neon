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
            .multi_threaded()
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
        auto& LoadedPacks = m_Manager->GetPacks();
        auto  PackIter    = LoadedPacks.begin();
        if (PackIter != LoadedPacks.end())
        {
            std::vector<std::future<void>> AsyncFlush;
            // Flush others in seperate threads
            for (auto NextPack = std::next(PackIter); NextPack != LoadedPacks.end(); NextPack++)
            {
                AsyncFlush.emplace_back(
                    std::async([NextPack]
                               { NextPack->second->Flush(); }));
            }
            // Try to flush the first one in current thread
            PackIter->second->Flush();
        }
    }
} // namespace Neon::Module