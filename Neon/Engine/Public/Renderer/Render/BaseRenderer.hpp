#pragma once

#include <RHI/Resource/Common.hpp>

#include <Scene/Scene.hpp>
#include <Runtime/GameEngine.hpp>
#include <flecs/flecs.h>

namespace Neon::RHI
{
    class IGraphicsCommandList;
}

namespace Neon::Renderer
{
    class IRenderer
    {
    public:
        IRenderer() = default;

        NEON_CLASS_NO_COPYMOVE(IRenderer);

        virtual ~IRenderer() = default;

        /// <summary>
        /// Render the scene using the given command list.
        /// </summary>
        virtual void Render(
            RHI::GpuResourceHandle     CameraBuffer,
            RHI::IGraphicsCommandList* CommandList) = 0;
    };
} // namespace Neon::Renderer