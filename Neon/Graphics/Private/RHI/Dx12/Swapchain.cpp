#include <GraphicsPCH.hpp>
#include <Core/Neon.hpp>

#include <Private/RHI/Dx12/Device.hpp>
#include <Private/RHI/Dx12/Swapchain.hpp>

#include <Math/Colors.hpp>
#include <RHI/Commands/CommandsContext.hpp>

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
        m_BudgetManager(this)
    {
        CreateSwapchain(Desc);
        ResizeBackbuffers(Desc.FramesInFlight);
    }

    Dx12Swapchain::~Dx12Swapchain()
    {
        m_Swapchain->SetFullscreenState(FALSE, nullptr);
    }

    //

    void Dx12Swapchain::PrepareFrame()
    {
        m_BudgetManager.NewFrame();
        uint32_t FrameIndex = m_BudgetManager.GetFrameIndex();

        TCommandContext<CommandQueueType::Graphics> CtxBatch(this);

        auto Context = CtxBatch.Append();

        auto Rtv = m_RenderTargets.GetCpuHandle(FrameIndex);

        auto StateManager = GetStateManager();
        StateManager->TransitionResource(&m_BackBuffers[FrameIndex], BitMask_Or(EResourceState::RenderTarget));
        StateManager->FlushBarriers(Context);

        m_Time += 0.008f;
        Color4 Color{
            sin(2.f * m_Time + 1.f) / 2.f + .5f,
            sin(3.f * m_Time + 2.f) / 2.f + .5f,
            sin(5.f * m_Time + 3.f) / 2.f + .5f,
            1.0f
        };
        Context->ClearRtv(Rtv, Color);
        Context->SetRenderTargets(Rtv, 1);

        StateManager->TransitionResource(&m_BackBuffers[FrameIndex], MResourceState_Common);
        StateManager->FlushBarriers(Context);

        //

        struct Vertex
        {
            DX::XMFLOAT4 Position;
            DX::XMFLOAT4 Color;
        };

        auto Buffer = IBuffer::Create(this, { .Size = sizeof(Vertex) });
        auto Upload = IUploadBuffer::Create(this, { .Size = sizeof(Vertex) });

        auto Vertices        = Upload->Map<Vertex>();
        Vertices[0].Position = { -0.5f, -0.5f, 0.0f, 1.0f };
        Vertices[0].Color    = { 1.0f, 0.0f, 0.0f, 1.0f };
        Upload->Unmap();

        CtxBatch.Upload();
    }

    void Dx12Swapchain::Present()
    {
        m_BudgetManager.EndFrame();
        ThrowIfFailed(m_Swapchain->Present(1, 0));
    }

    void Dx12Swapchain::Resize(
        const Size2I& Size)
    {
        m_BudgetManager.IdleGPU();
    }

    //

    void Dx12Swapchain::CreateSwapchain(
        const InitDesc& Desc)
    {
        auto DxgiFactory = Dx12RenderDevice::Get()->GetDxgiFactory();
        auto WindowSize  = Desc.Window->GetSize();

        Win32::ComPtr<IDXGIFactory2> DxgiFactory2;

        auto GraphicsQueue = m_BudgetManager.GetQueueManager()->GetGraphics()->Queue.Get();

        if (SUCCEEDED(DxgiFactory->QueryInterface(IID_PPV_ARGS(&DxgiFactory2))))
        {
            Win32::ComPtr<IDXGISwapChain1> Swapchain;

            DXGI_SWAP_CHAIN_DESC1 SwapchainDesc{
                .Width      = UINT(WindowSize.Width()),
                .Height     = UINT(WindowSize.Height()),
                .Format     = SwapchainFormat,
                .SampleDesc = {
                    .Count   = Desc.Sample.Count,
                    .Quality = Desc.Sample.Quality,
                },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = Desc.FramesInFlight,
                .SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD,
                .Flags       = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT
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
                    .Format = SwapchainFormat },
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
        size_t NewSize)
    {
        m_BackBuffers.clear();
        m_BackBuffers.reserve(NewSize);
        m_RenderTargets = Views::RenderTarget(
            NewSize,
            m_BudgetManager.GetDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false));

        for (size_t i = 0; i < NewSize; ++i)
        {
            Win32::ComPtr<ID3D12Resource> BackBuffer;
            ThrowIfFailed(m_Swapchain->GetBuffer(UINT(i), IID_PPV_ARGS(&BackBuffer)));
            auto& Buffer = m_BackBuffers.emplace_back(this, std::move(BackBuffer), D3D12_RESOURCE_STATE_PRESENT);

            m_RenderTargets.Bind(
                &Buffer,
                nullptr,
                i);
        }

        m_BudgetManager.ResizeFrames(NewSize);
    }
} // namespace Neon::RHI