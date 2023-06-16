#include <EnginePCH.hpp>
#include <Module/Graphics.hpp>
#include <Module/Window.hpp>
#include <Renderer/RG/RG.hpp>

namespace Neon::Module
{
    Graphics::Graphics(
        Runtime::DefaultGameEngine* Engine,
        const Config::EngineConfig& Config,
        Window*                     WindowModule)
    {
        auto& GraphicsConfig = Config.Graphics;

        RHI::ISwapchain::InitDesc Desc{
            .Window         = WindowModule->GetWindow(),
            .RefreshRate    = { GraphicsConfig.RefreshRate.Numerator, GraphicsConfig.RefreshRate.Denominator },
            .Sample         = { GraphicsConfig.Sample.Count, GraphicsConfig.Sample.Quality },
            .FramesInFlight = GraphicsConfig.FramesInFlight,
        };
        m_Swapchain.reset(RHI::ISwapchain::Create(Desc));

        m_OnWindowSizeChanged.Attach(
            WindowModule->OnWindowSizeChanged(),
            [this](const Size2I& Extent)
            { m_Swapchain->Resize(Extent); });

        //

        m_RenderGraph = std::make_unique<RG::RenderGraph>(m_Swapchain.get());
    }

    RHI::ISwapchain* Graphics::GetSwapchain() noexcept
    {
        return m_Swapchain.get();
    }

    RG::RenderGraph* Graphics::GetRenderGraph() noexcept
    {
        return m_RenderGraph.get();
    }

    void Graphics::PreRender()
    {
        m_Swapchain->PrepareFrame();
    }

    void Graphics::Render()
    {
        m_RenderGraph->Run();
    }

    void Graphics::PostRender()
    {
        m_Swapchain->Present();
    }
} // namespace Neon::Module