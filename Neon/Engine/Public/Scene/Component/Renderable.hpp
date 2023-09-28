#pragma once

#include <Scene/Component/Component.hpp>

namespace Neon::Scene::Component
{
    struct Renderable
    {
        friend class GPUScene;
        NEON_COMPONENT_SERIALIZE_IMPL
        {
        }

        NEON_EXPORT_FLECS(Renderable, "Renderable")
        {
            NEON_COMPONENT_SERIALIZE(Renderable);
        }

        constexpr operator bool() const noexcept
        {
            return InstanceId != std::numeric_limits<uint32_t>::max();
        }

    private:
        uint32_t InstanceId = std::numeric_limits<uint32_t>::max();
    };
} // namespace Neon::Scene::Component