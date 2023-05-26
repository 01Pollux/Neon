#pragma once

#include <World/World.hpp>
#include <RHI/Swapchain.hpp>
#include <Config/Engine.hpp>

#include <Module/Window.hpp>

namespace Neon::Module
{
    class Window;

    class Graphics
    {
    public:
        Graphics(
            Neon::World&                World,
            const Config::EngineConfig& Config,
            Window*                     WindowModule);

        NEON_CLASS_NO_COPYMOVE(Graphics);

        ~Graphics();

        /// <summary>
        /// Get the swapchain.
        /// </summary>
        RHI::ISwapchain* GetSwapchain() const noexcept;

    private:
        UPtr<RHI::ISwapchain> m_Swapchain;

        Signals::SHOnWindowSizeChanged m_OnWindowSizeChanged;
    };
} // namespace Neon::Module