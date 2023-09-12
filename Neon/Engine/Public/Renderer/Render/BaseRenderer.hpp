#pragma once

#include <RHI/Resource/Common.hpp>
#include <Runtime/GameEngine.hpp>

namespace Neon::RHI
{
    class ICommandList;
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
            RHI::GpuResourceHandle CameraBuffer,
            RHI::ICommandList*     CommandList) = 0;
    };
} // namespace Neon::Renderer