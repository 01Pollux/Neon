#include <GraphicsPCH.hpp>
#include <Core/Neon.hpp>

#include <Private/RHI/Dx12/Device.hpp>
#include <Private/RHI/Dx12/Swapchain.hpp>
#include <Private/RHI/Dx12/Resource/Common.hpp>

#include <Math/Colors.hpp>
#include <RHI/Commands/Context.hpp>

#include <Window/Window.hpp>

namespace Neon::RHI
{
    ISwapchain* ISwapchain::Create(
        const InitDesc& Desc)
    {
        return NEON_NEW Dx12Swapchain(Desc);
    }

    Dx12Swapchain::Dx12Swapchain(
        const InitDesc& Desc) :
        m_BudgetManager(this),
        m_WindowApp(Desc.Window),
        m_BackbufferFormat(Desc.BackbufferFormat),
        m_Size(Desc.Window->GetSize().get())
    {
        CreateSwapchain(Desc);
        ResizeBackbuffers(Desc.FramesInFlight);
    }

    Dx12Swapchain::~Dx12Swapchain()
    {
        m_Swapchain->SetFullscreenState(FALSE, nullptr);

        auto Allocator = m_BudgetManager.GetDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false);
        if (m_RenderTargets)
        {
            Allocator->Free(m_RenderTargets.GetHandle());
        }
        m_BudgetManager.Shutdown();
    }

    //

    void Dx12Swapchain::PrepareFrame()
    {
        m_BudgetManager.NewFrame();
    }

    void Dx12Swapchain::Present()
    {
        m_BudgetManager.EndFrame();
        ThrowIfFailed(m_Swapchain->Present(1, 0));
    }

    const Size2I& Dx12Swapchain::GetSize()
    {
        return m_Size;
    }

    EResourceFormat Dx12Swapchain::GetFormat()
    {
        return m_BackbufferFormat;
    }

    IGpuResource* Dx12Swapchain::GetBackBuffer()
    {
        return &m_BackBuffers[m_BudgetManager.GetFrameIndex()];
    }

    CpuDescriptorHandle Dx12Swapchain::GetBackBufferView()
    {
        return m_RenderTargets.GetCpuHandle(m_BudgetManager.GetFrameIndex());
    }

    void Dx12Swapchain::Resize(
        const Size2I&   Size,
        EResourceFormat NewFormat)
    {
        if (NewFormat == EResourceFormat::Unknown)
        {
            NewFormat = m_BackbufferFormat;
        }
        if (m_BackbufferFormat == NewFormat && m_Size == Size)
        {
            return;
        }

        m_Size             = Size;
        m_BackbufferFormat = NewFormat;

        uint32_t BackbufferCount = uint32_t(m_BackBuffers.size());
        m_BackBuffers.clear();
        m_BackBuffers.reserve(BackbufferCount);

        auto Allocator = GetDescriptorHeapManager(DescriptorType::RenderTargetView, false);
        if (m_RenderTargets)
        {
            Allocator->Free(m_RenderTargets.GetHandle());
        }

        m_BudgetManager.IdleGPU();

        ThrowIfFailed(m_Swapchain->ResizeBuffers(
            0,
            Size.Width(),
            Size.Height(),
            CastFormat(m_BackbufferFormat),
            0));

        m_RenderTargets = Views::RenderTarget(
            GetDescriptorHeapManager(DescriptorType::RenderTargetView, false),
            BackbufferCount);

        for (uint32_t i = 0; i < BackbufferCount; ++i)
        {
            Win32::ComPtr<ID3D12Resource> BackBuffer;
            ThrowIfFailed(m_Swapchain->GetBuffer(UINT(i), IID_PPV_ARGS(&BackBuffer)));
            auto& Buffer = m_BackBuffers.emplace_back(this, std::move(BackBuffer), D3D12_RESOURCE_STATE_PRESENT);

            m_RenderTargets.Bind(
                &Buffer,
                nullptr,
                i);
        }

        m_BudgetManager.ResetFrameIndex();
        m_BudgetManager.IdleGPU();
    }

    //

    void Dx12Swapchain::CreateSwapchain(
        const InitDesc& Desc)
    {
        auto WindowSizeFuture = Desc.Window->GetSize();

        auto DxgiFactory = Dx12RenderDevice::Get()->GetDxgiFactory();

        Win32::ComPtr<IDXGIFactory2> DxgiFactory2;

        auto GraphicsQueue = m_BudgetManager.GetQueueManager()->GetGraphics()->Queue.Get();

        auto WindowSize = WindowSizeFuture.get();

        if (SUCCEEDED(DxgiFactory->QueryInterface(IID_PPV_ARGS(&DxgiFactory2))))
        {
            Win32::ComPtr<IDXGISwapChain1> Swapchain;

            DXGI_SWAP_CHAIN_DESC1 SwapchainDesc{
                .Width      = UINT(WindowSize.Width()),
                .Height     = UINT(WindowSize.Height()),
                .Format     = CastFormat(m_BackbufferFormat),
                .SampleDesc = {
                    .Count   = Desc.Sample.Count,
                    .Quality = Desc.Sample.Quality,
                },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = Desc.FramesInFlight,
                .SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD,
                .Flags       = 0
            };

            DXGI_SWAP_CHAIN_FULLSCREEN_DESC FullscreenDesc{
                .RefreshRate = {
                    .Numerator   = Desc.RefreshRate.Numerator,
                    .Denominator = Desc.RefreshRate.Denominator,
                },
                .Windowed = TRUE,
            };

            ThrowIfFailed(DxgiFactory2->CreateSwapChainForHwnd(
                GraphicsQueue,
                HWND(Desc.Window->GetPlatformHandle()),
                &SwapchainDesc,
                &FullscreenDesc,
                nullptr,
                Swapchain.GetAddressOf()));

            ThrowIfFailed(Swapchain.As(&m_Swapchain));
        }
        else
        {
            Win32::ComPtr<IDXGISwapChain> Swapchain;

            DXGI_SWAP_CHAIN_DESC SCDesc{
                .BufferDesc = {
                    .Width       = UINT(WindowSize.Width()),
                    .Height      = UINT(WindowSize.Height()),
                    .RefreshRate = {
                        .Numerator   = Desc.RefreshRate.Numerator,
                        .Denominator = Desc.RefreshRate.Denominator,
                    },
                    .Format = CastFormat(m_BackbufferFormat) },
                .SampleDesc = {
                    .Count   = Desc.Sample.Count,
                    .Quality = Desc.Sample.Quality,
                },
                .BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount  = Desc.FramesInFlight,
                .OutputWindow = HWND(Desc.Window->GetPlatformHandle()),
                .Windowed     = TRUE,
                .SwapEffect   = DXGI_SWAP_EFFECT_FLIP_DISCARD,
                .Flags        = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT
            };

            DxgiFactory->CreateSwapChain(
                GraphicsQueue,
                &SCDesc,
                Swapchain.GetAddressOf());

            ThrowIfFailed(Swapchain.As(&m_Swapchain));
        }

        m_Swapchain->SetMaximumFrameLatency(Desc.FramesInFlight);
    }

    void Dx12Swapchain::ResizeBackbuffers(
        uint32_t NewSize)
    {
        m_BackBuffers.clear();
        m_BackBuffers.reserve(NewSize);
        m_BudgetManager.ResizeFrames(NewSize);

        auto Allocator = GetDescriptorHeapManager(DescriptorType::RenderTargetView, false);
        if (m_RenderTargets)
        {
            Allocator->Free(m_RenderTargets.GetHandle());
        }

        m_RenderTargets = Views::RenderTarget(
            GetDescriptorHeapManager(DescriptorType::RenderTargetView, false),
            NewSize);

        for (uint32_t i = 0; i < NewSize; ++i)
        {
            Win32::ComPtr<ID3D12Resource> BackBuffer;
            ThrowIfFailed(m_Swapchain->GetBuffer(UINT(i), IID_PPV_ARGS(&BackBuffer)));
            auto& Buffer = m_BackBuffers.emplace_back(this, std::move(BackBuffer), D3D12_RESOURCE_STATE_PRESENT);

            m_RenderTargets.Bind(
                &Buffer,
                nullptr,
                i);
        }
    }
} // namespace Neon::RHI