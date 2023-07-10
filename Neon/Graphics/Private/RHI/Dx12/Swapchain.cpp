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
    ISwapchain* ISwapchain::Get()
    {
        return IRenderDevice::Get()->GetSwapchain();
    }

    void Dx12Swapchain::PostInitialize(
        const SwapchainCreateDesc& Desc)
    {
        ResizeBackbuffers(Desc.FramesInFlight);
    }

    void Dx12Swapchain::Shutdown()
    {
        m_Swapchain->SetFullscreenState(FALSE, nullptr);

        m_BackBuffers.clear();
        m_FrameManager = nullptr;
    }

    //

    Dx12Swapchain::Dx12Swapchain(
        Windowing::IWindowApp*     Window,
        const SwapchainCreateDesc& Desc) :
        m_WindowApp(Window),
        m_BackbufferFormat(Desc.BackbufferFormat),
        m_Size(m_WindowApp->GetSize().get())
    {
        m_FrameManager = std::make_unique<FrameManager>();
        CreateSwapchain(Desc);
    }

    //

    void Dx12Swapchain::PrepareFrame()
    {
        m_FrameManager->NewFrame();
    }

    void Dx12Swapchain::Present()
    {
        m_FrameManager->EndFrame();
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
        return &m_BackBuffers[m_FrameManager->GetFrameIndex()];
    }

    CpuDescriptorHandle Dx12Swapchain::GetBackBufferView()
    {
        return m_RenderTargets.GetCpuHandle(m_FrameManager->GetFrameIndex());
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
        // Clearing the backbuffer vector will release the resources into the 'garbage collector' for current frame
        // and will be released after the frame is finished or the call to IdleGPU()
        m_BackBuffers.clear();
        m_BackBuffers.reserve(BackbufferCount);

        m_FrameManager->IdleGPU();

        auto Allocator = IStaticDescriptorHeap::Get(DescriptorType::RenderTargetView);
        if (m_RenderTargets)
        {
            Allocator->Free(m_RenderTargets.GetHandle());
        }

        m_RenderTargets = Allocator->Allocate(BackbufferCount);

        ThrowIfFailed(m_Swapchain->ResizeBuffers(
            0,
            Size.Width(),
            Size.Height(),
            CastFormat(m_BackbufferFormat),
            0));

        for (uint32_t i = 0; i < BackbufferCount; ++i)
        {
            Win32::ComPtr<ID3D12Resource> BackBuffer;
            ThrowIfFailed(m_Swapchain->GetBuffer(UINT(i), IID_PPV_ARGS(&BackBuffer)));
            auto& Buffer = m_BackBuffers.emplace_back(std::move(BackBuffer), D3D12_RESOURCE_STATE_PRESENT);
            RHI::RenameObject(Buffer.GetResource(), STR("Swapchain Backbuffer {}"), i);

            m_RenderTargets.Bind(
                &Buffer,
                nullptr,
                i);
        }

        m_FrameManager->ResetFrameIndex();
        m_FrameManager->IdleGPU();
    }

    //

    void Dx12Swapchain::CreateSwapchain(
        const SwapchainCreateDesc& Desc)
    {
        auto DxgiFactory = Dx12RenderDevice::Get()->GetDxgiFactory();

        Win32::ComPtr<IDXGIFactory2> DxgiFactory2;

        auto GraphicsQueue = m_FrameManager->GetQueueManager()->GetGraphics()->Queue.Get();

        if (SUCCEEDED(DxgiFactory->QueryInterface(IID_PPV_ARGS(&DxgiFactory2))))
        {
            Win32::ComPtr<IDXGISwapChain1> Swapchain;

            DXGI_SWAP_CHAIN_DESC1 SwapchainDesc{
                .Width      = UINT(m_Size.Width()),
                .Height     = UINT(m_Size.Height()),
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
                HWND(m_WindowApp->GetPlatformHandle()),
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
                    .Width       = UINT(m_Size.Width()),
                    .Height      = UINT(m_Size.Height()),
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
                .OutputWindow = HWND(m_WindowApp->GetPlatformHandle()),
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
    }

    void Dx12Swapchain::ResizeBackbuffers(
        uint32_t NewSize)
    {
        m_BackBuffers.clear();
        m_BackBuffers.reserve(NewSize);
        m_FrameManager->ResizeFrames(NewSize);

        auto Allocator = IStaticDescriptorHeap::Get(DescriptorType::RenderTargetView);
        if (m_RenderTargets)
        {
            Allocator->Free(m_RenderTargets.GetHandle());
        }
        m_RenderTargets = Allocator->Allocate(NewSize);

        for (uint32_t i = 0; i < NewSize; ++i)
        {
            Win32::ComPtr<ID3D12Resource> BackBuffer;
            ThrowIfFailed(m_Swapchain->GetBuffer(UINT(i), IID_PPV_ARGS(&BackBuffer)));
            auto& Buffer = m_BackBuffers.emplace_back(std::move(BackBuffer), D3D12_RESOURCE_STATE_PRESENT);
            RHI::RenameObject(Buffer.GetResource(), STR("Swapchain Backbuffer {}"), i);

            m_RenderTargets.Bind(
                &Buffer,
                nullptr,
                i);
        }
    }
} // namespace Neon::RHI