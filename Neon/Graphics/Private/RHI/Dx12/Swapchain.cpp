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
        const InitDesc& Desc) :
        m_CommandQueue(std::make_unique<Dx12CommandQueue>(CommandQueueType::Graphics))
    {
        m_CommandAllocators.resize(Desc.FramesInFlight);
        TestCommandList();

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

    Dx12Swapchain::~Dx12Swapchain()
    {
        Win32::ComPtr<ID3D12Fence> Fence;
        Dx12RenderDevice::Get()->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence));

        m_CommandQueue->Get()->Signal(Fence.Get(), 1);

        Win32::WinHandlePtr Handle(CreateEvent(nullptr, FALSE, FALSE, nullptr));
        Fence->SetEventOnCompletion(1, Handle.Get());

        m_Swapchain->SetFullscreenState(FALSE, nullptr);
    }

    //

    void Dx12Swapchain::PrepareFrame()
    {
        static float Time = float(rand());

        HANDLE PresentHandle = m_Swapchain->GetFrameLatencyWaitableObject();
        WaitForSingleObject(PresentHandle, INFINITE);

        uint32_t FrameIndex = m_Swapchain->GetCurrentBackBufferIndex();
        m_CommandAllocators[FrameIndex]->Reset();
        m_CommandList->Reset(m_CommandAllocators[FrameIndex].get());

        Win32::ComPtr<ID3D12Resource> Buffer;
        m_Swapchain->GetBuffer(FrameIndex, IID_PPV_ARGS(&Buffer));

        auto& Rtv = m_RenderTargets[Buffer.Get()];
        if (!Rtv.ptr)
        {
            ID3D12DescriptorHeap*      Heap;
            D3D12_DESCRIPTOR_HEAP_DESC Desc{
                .Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
                .NumDescriptors = 1
            };

            auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();
            Dx12Device->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&Heap));

            Rtv = Heap->GetCPUDescriptorHandleForHeapStart();
            Dx12RenderDevice::Get()->GetDevice()->CreateRenderTargetView(Buffer.Get(), nullptr, Rtv);
        }

        auto CommandList = m_CommandList->Get();

        {
            // transition buffer resource to render target state
            const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                Buffer.Get(),
                D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
            CommandList->ResourceBarrier(1, &barrier);
        }

        Time += 0.008f;
        float Color[]{
            sin(2.f * Time + 1.f) / 2.f + .5f,
            sin(3.f * Time + 2.f) / 2.f + .5f,
            sin(5.f * Time + 3.f) / 2.f + .5f,
            1.0f
        };
        CommandList->ClearRenderTargetView(Rtv, Color, 0, nullptr);
        CommandList->OMSetRenderTargets(1, &Rtv, FALSE, nullptr);

        {
            // transition buffer resource to render target state
            const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                Buffer.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            CommandList->ResourceBarrier(1, &barrier);
        }

        CommandList->Close();
        auto Commands = static_cast<ID3D12CommandList*>(m_CommandList->Get());
        m_CommandQueue->Get()->ExecuteCommandLists(1, &Commands);

        ThrowIfFailed(m_Swapchain->Present(1, 0));
    }

    void Dx12Swapchain::Present()
    {
    }

    void Dx12Swapchain::Resize(
        const Size2I& Size)
    {
    }

    //

    void Dx12Swapchain::TestCommandList()
    {
        for (auto& Allocator : m_CommandAllocators)
        {
            Allocator = std::make_unique<Dx12CommandAllocator>(CommandQueueType::Graphics);
        }
        m_CommandList = std::make_unique<Dx12CommandList>(m_CommandAllocators[0].get(), CommandQueueType::Graphics);
    }
} // namespace Neon::RHI