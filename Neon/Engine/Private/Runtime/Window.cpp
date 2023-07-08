#include <EnginePCH.hpp>
#include <Runtime/Window.hpp>
#include <Runtime/GameEngine.hpp>

#include <RHI/Device.hpp>
#include <RHI/Swapchain.hpp>

namespace Neon::Runtime
{
    EngineWindow::EngineWindow(
        const Config::EngineConfig& Config)
    {
        auto& WindowConfig = Config.Window;

        Windowing::MWindowStyle Style;
        if (WindowConfig.WithCloseButton)
        {
            Style.Set(Windowing::EWindowStyle::Close);
        }
        if (WindowConfig.CanResize)
        {
            Style.Set(Windowing::EWindowStyle::Resize);
        }
        if (WindowConfig.HasTitleBar)
        {
            Style.Set(Windowing::EWindowStyle::TitleBar);
        }
        if (WindowConfig.Windowed)
        {
            Style.Set(Windowing::EWindowStyle::Windowed);
        }
        if (WindowConfig.Fullscreen)
        {
            Style.Set(Windowing::EWindowStyle::Fullscreen);
        }

        m_Window.reset(Windowing::IWindowApp::Create(WindowConfig.Title, WindowConfig.Size, Style, WindowConfig.StartInMiddle));

        // Create the render device and swapchain

        auto& RendererConfig = Config.Renderer;
        RHI::IRenderDevice::Create(m_Window.get(), RendererConfig.Device, RendererConfig.Swapchain);
    }

    EngineWindow::~EngineWindow()
    {
        RHI::IRenderDevice::Destroy();
    }

    int EngineWindow::GetExitCode() const noexcept
    {
        return m_ExitCode;
    }

    Windowing::IWindowApp* EngineWindow::GetWindow() const noexcept
    {
        return m_Window.get();
    }

    bool EngineWindow::Run()
    {
        namespace WinEvents = Windowing::Events;

        Windowing::Event Msg;
        while (m_Window->PeekEvent(Msg))
        {
            std::visit(
                VariantVisitor{
                    [this](const WinEvents::SizeChanged& SizeMsg)
                    {
                        RHI::ISwapchain::Get()->Resize(SizeMsg.NewSize);
                        OnWindowSizeChanged().Broadcast(SizeMsg.NewSize);
                    } },
                Msg);
        }
        return m_Window->IsRunning();
    }
} // namespace Neon::Runtime