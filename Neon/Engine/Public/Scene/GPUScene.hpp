#pragma once

#include <Scene/GPUTransformManager.hpp>

namespace Neon::Scene
{
    class GPUScene
    {
    public:
        const auto& GetGPUTransform() const noexcept
        {
            return m_TransformManager;
        }

        auto& GetGPUTransform() noexcept
        {
            return m_TransformManager;
        }

    private:
        GPUTransformManager m_TransformManager;
    };
} // namespace Neon::Scene