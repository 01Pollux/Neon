#include <GraphicsPCH.hpp>
#include <Core/Neon.hpp>

#include <Private/RHI/Dx12/Device.hpp>
#include <Private/RHI/Dx12/Swapchain.hpp>

#include <Private/RHI/Dx12/Commands/CommandList.hpp>
#include <Math/Colors.hpp>
#include <RHI/Resource/Common.hpp>
#include <RHI/Commands/CommandList.hpp>
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
        m_CommandQueue(std::make_unique<Dx12CommandQueue>(CommandQueueType::Graphics)),
        m_FrameFence(IFence::Create())
    {
        // TODO remove
        srand(time(nullptr));
        CreateSwapchain(Desc);
        ResizeBackbuffers(Desc.FramesInFlight);
    }

    Dx12Swapchain::~Dx12Swapchain()
    {
        m_FrameFence->SignalGPU(m_CommandQueue.get(), m_FenceValue);
        m_FrameFence->WaitCPU(m_FenceValue);

        m_Swapchain->SetFullscreenState(FALSE, nullptr);
    }

    //

    void Dx12Swapchain::PrepareFrame()
    {
        static float Time = float(rand());

        m_FrameFence->CmpWaitCPU(m_FenceValue);
        uint32_t FrameIndex = uint32_t(m_FenceValue % m_RenderTargets.size());

        TCommandContext<CommandQueueType::Graphics> Context(m_CommandQueue.get());

        auto& Rtv = m_RenderTargets[FrameIndex];

        auto StateManager = IRenderDevice::Get()->GetStateManager();
        StateManager->TransitionResource(&m_BackBuffers[FrameIndex], BitMask_Or(EResourceState::RenderTarget));
        StateManager->FlushBarriers(Context.Get());

        Time += 0.008f;
        Color4 Color{
            sin(2.f * Time + 1.f) / 2.f + .5f,
            sin(3.f * Time + 2.f) / 2.f + .5f,
            sin(5.f * Time + 3.f) / 2.f + .5f,
            1.0f
        };
        Context->ClearRtv({ Rtv.ptr }, Color);
        Context->SetRenderTargets({ Rtv.ptr }, 1);

        StateManager->TransitionResource(&m_BackBuffers[FrameIndex], MResourceState_Common);
        StateManager->FlushBarriers(Context.Get());

        Context.Upload();
    }

    void Dx12Swapchain::Present()
    {
        m_FrameFence->SignalGPU(m_CommandQueue.get(), ++m_FenceValue);
        ThrowIfFailed(m_Swapchain->Present(1, 0));
    }

    void Dx12Swapchain::Resize(
        const Size2I& Size)
    {
    }

    //

    void Dx12Swapchain::CreateSwapchain(
        const InitDesc& Desc)
    {
        auto DxgiFactory = Dx12RenderDevice::Get()->GetDxgiFactory();
        auto WindowSize  = Desc.Window->GetSize();

        Win32::ComPtr<IDXGIFactory2> DxgiFactory2;

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
                m_CommandQueue->Get(),
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
                m_CommandQueue->Get(),
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
        m_RenderTargets.resize(NewSize);

        ID3D12DescriptorHeap*      Heap;
        D3D12_DESCRIPTOR_HEAP_DESC Desc{
            .Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            .NumDescriptors = 1
        };

        for (size_t i = 0; i < NewSize; ++i)
        {
            Win32::ComPtr<ID3D12Resource> BackBuffer;
            ThrowIfFailed(m_Swapchain->GetBuffer(UINT(i), IID_PPV_ARGS(&BackBuffer)));
            auto& Buffer = m_BackBuffers.emplace_back(std::move(BackBuffer), D3D12_RESOURCE_STATE_PRESENT);

            // TODO

            auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();
            Dx12Device->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&Heap));

            m_RenderTargets[i] = Heap->GetCPUDescriptorHandleForHeapStart();
            Dx12RenderDevice::Get()->GetDevice()->CreateRenderTargetView(Buffer.GetResource(), nullptr, m_RenderTargets[i]);
        }
    }
} // namespace Neon::RHI