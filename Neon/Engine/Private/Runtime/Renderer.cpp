#include <EnginePCH.hpp>
#include <Runtime/Renderer.hpp>
#include <Runtime/GameEngine.hpp>
#include <Runtime/Window.hpp>
#include <Renderer/RG/RG.hpp>

namespace Neon::Runtime
{
    EngineRenderer::EngineRenderer(
        Runtime::DefaultGameEngine* Engine,
        const Config::EngineConfig& Config)
    {
        m_Window = Engine->QueryInterface<EngineWindow>();

        auto& GraphicsConfig = Config.Renderer;

        RHI::ISwapchain::InitDesc Desc{
            .Window         = m_Window->GetWindow(),
            .RefreshRate    = { GraphicsConfig.RefreshRate.Numerator, GraphicsConfig.RefreshRate.Denominator },
            .Sample         = { GraphicsConfig.Sample.Count, GraphicsConfig.Sample.Quality },
            .FramesInFlight = GraphicsConfig.FramesInFlight,
        };
        m_Swapchain.reset(RHI::ISwapchain::Create(Desc));

        m_OnWindowSizeChanged.Attach(
            m_Window->OnWindowSizeChanged(),
            [this](const Size2I& Extent)
            { m_Swapchain->Resize(Extent); });

        //

        m_RenderGraph = std::make_unique<RG::RenderGraph>(m_Swapchain.get());
    }

    RHI::ISwapchain* EngineRenderer::GetSwapchain() noexcept
    {
        return m_Swapchain.get();
    }

    RG::RenderGraph* EngineRenderer::GetRenderGraph() noexcept
    {
        return m_RenderGraph.get();
    }

    void EngineRenderer::PreRender()
    {
        m_WindowIsVisible = m_Window->GetWindow()->IsVisible().get();
        if (IsRendering())
        {
            m_Swapchain->PrepareFrame();
        }
    }

    void EngineRenderer::Render()
    {
        if (IsRendering())
        {
            m_RenderGraph->Run();
        }
    }

    void EngineRenderer::PostRender()
    {
        if (IsRendering())
        {
            m_Swapchain->Present();
        }
    }

    bool EngineRenderer::IsRendering() const noexcept
    {
        return m_WindowIsVisible;
    }
} // namespace Neon::Runtime