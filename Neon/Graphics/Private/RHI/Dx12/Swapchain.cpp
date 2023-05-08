#include <GraphicsPCH.hpp>
#include <Core/Neon.hpp>

#include <Private/RHI/Dx12/Device.hpp>
#include <Private/RHI/Dx12/Swapchain.hpp>

#include <Window/Window.hpp>

namespace Neon::RHI
{
    ISwapchain* ISwapchain::Create(
        const InitDesc& Desc)
    {
        return NEON_NEW Dx12Swapchain(Desc);
    }

    Dx12Swapchain::Dx12Swapchain(
        const InitDesc& Desc)
    {
        auto DxgiFactory = Dx12RenderDevice::Get()->GetDxgiFactory();

        Win32::ComPtr<IDXGIFactory2> DxgiFactory2;
        if (SUCCEEDED(DxgiFactory->QueryInterface(IID_PPV_ARGS(&DxgiFactory2))))
        {
            ThrowIfFailed(DxgiFactory2->CreateSwapChainForHwnd(
                nullptr,
                HWND(Desc.Window->GetPlatformHandle()),
                nullptr,
                nullptr,
                nullptr,
                nullptr));
        }
        else
        {
            auto WindowSize = Desc.Window->GetSize();

            DXGI_SWAP_CHAIN_DESC SCDesc{
                .BufferDesc = {
                    .Width       = WindowSize.Width(),
                    .Height      = WindowSize.Height(),
                    .RefreshRate = {
                        .Numerator   = Desc.RefreshRate.Numerator,
                        .Denominator = Desc.RefreshRate.Denominator,
                    },
                    .Format = SwapchainFormat },
                .SampleDesc = {
                    .Count   = Desc.Sample.Count,
                    .Quality = Desc.Sample.Quality,
                },
                .BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount  = Desc.FramesInFlight,
                .OutputWindow = HWND(Desc.Window->GetPlatformHandle()),
                .Windowed     = TRUE,
                .Flags        = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT | DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
            };

            DxgiFactory->CreateSwapChain(
                nullptr,
                &SCDesc,
                m_Swapchain.GetAddressOf());
        }

        Win32::ComPtr<IDXGISwapChain2> Swapchain2;
        ThrowIfFailed(m_Swapchain->QueryInterface(IID_PPV_ARGS(&Swapchain2)));
        Swapchain2->SetMaximumFrameLatency(Desc.FramesInFlight);
    }

    void Dx12Swapchain::PrepareFrame()
    {
    }

    void Dx12Swapchain::Present()
    {
    }

    const Size2I& Dx12Swapchain::GetSize()
    {
        // TODO: insert return statement here
    }

    void Dx12Swapchain::Resize(const Size2I& Size)
    {
    }
} // namespace Neon::RHI