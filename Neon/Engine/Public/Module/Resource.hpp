#pragma once

#include <Resource/Manager.hpp>

namespace Neon::Runtime
{
    class DefaultGameEngine;
} // namespace Neon::Runtime

namespace Neon::Module
{
    class ResourceManager
    {
    public:
        ResourceManager(
            Runtime::DefaultGameEngine* Engine,
            Asset::IResourceManager*    Manager);

        /// <summary>
        /// Get resource manager.
        /// </summary>
        [[nodiscard]] Asset::IResourceManager* Get();

    private:
        UPtr<Asset::IResourceManager> m_Manager;
    };
} // namespace Neon::Module