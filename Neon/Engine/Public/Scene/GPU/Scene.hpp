#pragma once

#include <Scene/GPU/TransformManager.hpp>
#include <Scene/GPU/LightManager.hpp>

namespace Neon::Scene
{
    class GPUScene
    {
    public:
        const auto& GetTransformManager() const noexcept
        {
            return m_TransformManager;
        }

        auto& GetTransformManager() noexcept
        {
            return m_TransformManager;
        }

        const auto& GetLightManager() const noexcept
        {
            return m_LightManager;
        }

        auto& GetLightManager() noexcept
        {
            return m_LightManager;
        }

    private:
        GPUTransformManager m_TransformManager;
        GPULightManager     m_LightManager;
    };
} // namespace Neon::Scene