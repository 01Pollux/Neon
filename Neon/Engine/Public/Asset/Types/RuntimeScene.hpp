#pragma once

#include <Asset/Asset.hpp>
#include <Scene/RuntimeScene.hpp>

namespace Neon::Asset
{
    class RuntimeSceneAsset : public IAsset
    {
    public:
        class Handler;

        using IAsset::IAsset;

        /// <summary>
        /// Get the log tags.
        /// </summary>
        auto& GetScene() const noexcept
        {
            return m_Scene;
        }

    private:
        Scene::RuntimeScene m_Scene;
    };
} // namespace Neon::Asset