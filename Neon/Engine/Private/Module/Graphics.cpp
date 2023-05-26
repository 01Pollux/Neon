#include <EnginePCH.hpp>
#include <Module/Graphics.hpp>
#include <Module/Window.hpp>

#include <RHI/Device.hpp>

namespace Neon::Module
{
    Graphics::Graphics(
        Neon::World&                World,
        const Config::EngineConfig& Config,
        Window*                     WindowModule)
    {
        World.ModuleScope<Graphics>();

        RHI::IRenderDevice::CreateGlobal();

        auto& GraphicsConfig = Config.Graphics;

        RHI::ISwapchain::InitDesc Desc{
            .Window         = WindowModule->GetWindow(),
            .RefreshRate    = { GraphicsConfig.RefreshRate.Numerator, GraphicsConfig.RefreshRate.Denominator },
            .Sample         = { GraphicsConfig.Sample.Count, GraphicsConfig.Sample.Quality },
            .FramesInFlight = GraphicsConfig.FramesInFlight,
        };
        m_Swapchain.reset(RHI::ISwapchain::Create(Desc));

        World->system("Swapchain::PrepareFrame")
            .kind(flecs::PreFrame)
            .iter([this](flecs::iter& Iter)
                  { m_Swapchain->PrepareFrame(); });

        World->system("Swapchain::Present")
            .kind(flecs::PostFrame)
            .iter([this](flecs::iter& Iter)
                  { m_Swapchain->Present(); });

        // m_OnWindowSizeChanged.Attach(
        //     WindowModule->OnWindowSizeChanged(),
        //     [this](const Size2I& Extent)
        //     { m_Swapchain->Resize(Extent); });
    }

    Graphics::~Graphics()
    {
        RHI::IRenderDevice::DestroyGlobal();
    }

    RHI::ISwapchain* Graphics::GetSwapchain() const noexcept
    {
        return m_Swapchain.get();
    }
} // namespace Neon::Module