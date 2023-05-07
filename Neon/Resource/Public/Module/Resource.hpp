#pragma once

#include <Resource/Manager.hpp>
#include <World/World.hpp>

namespace Neon::Module
{
    class ResourceManager
    {
    public:
        ResourceManager(
            Neon::World&             World,
            Asset::IResourceManager* Manager);

        /// <summary>
        /// Get resource manager.
        /// </summary>
        [[nodiscard]] Asset::IResourceManager* Get();

    private:
        /// <summary>
        /// Called on ::OnLoad
        /// Flushes the pending asset packs.
        /// </summary>
        void FlushPacks(
            flecs::iter& Iter);

    private:
        UPtr<Asset::IResourceManager> m_Manager;
    };
} // namespace Neon::Module