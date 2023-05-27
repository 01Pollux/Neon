#pragma once

#include <Resource/Manager.hpp>

namespace Neon
{
    class DefaultGameEngine;
} // namespace Neon

namespace Neon::Module
{
    class ResourceManager
    {
    public:
        ResourceManager(
            DefaultGameEngine*       Engine,
            Asset::IResourceManager* Manager);

        /// <summary>
        /// Get resource manager.
        /// </summary>
        [[nodiscard]] Asset::IResourceManager* Get();

        /// <summary>
        /// Run the resource manager.
        /// </summary>
        void Run();

    private:
        /// <summary>
        /// Flushes the pending asset packs.
        /// </summary>
        void FlushPacks();

    private:
        UPtr<Asset::IResourceManager> m_Manager;
    };
} // namespace Neon::Module