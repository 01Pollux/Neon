#include <GraphicsPCH.hpp>
#include <Core/Neon.hpp>

#include <Private/RHI/Dx12/Device.hpp>
#include <Private/RHI/Dx12/Swapchain.hpp>
#include <Private/RHI/Dx12/Resource/Common.hpp>

#include <Math/Colors.hpp>
#include <RHI/Commands/Context.hpp>

#include <Window/Window.hpp>

//

#include <fstream>

#include <RHI/PipelineState.hpp>
#include <RHI/RootSignature.hpp>
#include <RHI/Resource/Views/Shader.hpp>

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
        m_BackbufferFormat(Desc.BackbufferFormat)
    {
        CreateSwapchain(Desc);
        ResizeBackbuffers(Desc.FramesInFlight);
    }

    Dx12Swapchain::~Dx12Swapchain()
    {
        m_PipelineState = nullptr;
        m_RootSignature = nullptr;
        m_Swapchain->SetFullscreenState(FALSE, nullptr);

        auto Allocator = m_BudgetManager.GetDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false);
        if (m_RenderTargets)
        {
            Allocator->Free(m_RenderTargets.GetHandle());
        }
        m_BudgetManager.IdleGPU();
    }

    //

    void Dx12Swapchain::PrepareFrame()
    {
        m_BudgetManager.NewFrame();

        struct VsInput
        {
            Vector2D Position;
        };

        Ptr<IUploadBuffer> m_Buffer;
        static bool        test = false;
        {
            if (!test)
            {
                test = true;

                std::stringstream Stream;
                std::ifstream     File(L"D:\\Dev\\Shader.hlsl");

                Stream << File.rdbuf();
                auto Text = Stream.str();

                ShaderCompileDesc Desc{
                    .Stage      = ShaderStage::Vertex,
                    .SourceCode = Text,
                    .EntryPoint = STR("VSMain")
                };

                auto VsShader = Ptr<IShader>(IShader::Create(Desc));

                Desc.Stage      = ShaderStage::Pixel;
                Desc.EntryPoint = STR("PSMain");

                auto PsShader = Ptr<IShader>(IShader::Create(Desc));

                m_RootSignature = IRootSignature::Create(
                    RootSignatureBuilder()
                        .Add32BitConstants<float>(0, 0)
                        .AddConstantBufferView(1, 0)
                        .SetFlags(ERootSignatureBuilderFlags::AllowInputLayout));

                PipelineStateBuilder<false> Builder{
                    .RootSignature     = m_RootSignature.get(),
                    .VertexShader      = VsShader.get(),
                    .PixelShader       = PsShader.get(),
                    .DepthStencilState = { .DepthEnable = false },
                    .PrimitiveTopology = PipelineStateBuilder<false>::Toplogy::Triangle,
                    .RTFormats         = { EResourceFormat::R8G8B8A8_UNorm },
                };

                VsShader->CreateInputLayout(Builder.InputLayout);

                m_PipelineState = IPipelineState::Create(Builder);
            }
        }

        //

        m_Buffer.reset(IUploadBuffer::Create(
            this,
            { .Size = sizeof(VsInput) * 4 + sizeof(uint16_t) * 6 }));

        auto Vertex = m_Buffer->Map<VsInput>();

        Vertex[0] = { Vector2D(-1.f, -1.f) };
        Vertex[1] = { Vector2D(-1.f, 1.f) };
        Vertex[2] = { Vector2D(1.f, -1.f) };
        Vertex[3] = { Vector2D(1.f, 1.f) };

        auto Input = m_Buffer->Map<uint16_t>(sizeof(VsInput) * 4);

        Input[0] = 0;
        Input[1] = 1;
        Input[2] = 2;
        Input[3] = 2;
        Input[4] = 1;
        Input[5] = 3;

        m_Buffer->Unmap();
        m_Buffer->Unmap();

        //

        uint32_t FrameIndex = m_BudgetManager.GetFrameIndex();

        TCommandContext<CommandQueueType::Graphics> CtxBatch(this);

        auto Context = CtxBatch.Append();

        auto Rtv = m_RenderTargets.GetCpuHandle(FrameIndex);

        auto StateManager = GetStateManager();
        StateManager->TransitionResource(&m_BackBuffers[FrameIndex], MResourceState::FromEnum(EResourceState::RenderTarget));
        StateManager->FlushBarriers(Context);

        Color4 Color = Colors::White;

        Context->ClearRtv(Rtv, Color);
        Context->SetRenderTargets(Rtv, 1);

        Context->SetRootSignature(m_RootSignature);
        Context->SetPipelineState(m_PipelineState);

        static float Time = 0.f;
        Time += 0.03f;
        Context->SetConstants(0, &Time, 1);

        Views::Vertex Vtx;
        Vtx.Append(m_Buffer.get(), 0, sizeof(VsInput), sizeof(VsInput) * 4);
        Context->SetVertexBuffer(0, Vtx);

        Views::Index Idx(m_Buffer.get(), sizeof(VsInput) * 4, sizeof(uint16_t) * 6);
        Context->SetIndexBuffer(Idx);

        Context->SetPrimitiveTopology(PrimitiveTopology::TriangleList);

        Context->SetViewport(ViewportF{
            .Width  = 1280.f,
            .Height = 720.f });

        Context->SetScissorRect(
            RectF(Vector2D::Zero, { 1280.f, 720.f }));

        Context->SetDynamicResourceView(
            RHI::ICommonCommandList::ViewType::Cbv,
            1,
            Colors::Green.data(),
            sizeof(float) * 4);

        Context->Draw(DrawIndexArgs{
            .IndexCountPerInstance = 6 });

        StateManager->TransitionResource(&m_BackBuffers[FrameIndex], MResourceState_Common);
        StateManager->FlushBarriers(Context);
    }

    void Dx12Swapchain::Present()
    {
        m_BudgetManager.EndFrame();
        ThrowIfFailed(m_Swapchain->Present(1, 0));
    }

    Windowing::IWindowApp* Dx12Swapchain::GetWindow()
    {
        return m_WindowApp;
    }

    EResourceFormat Dx12Swapchain::GetFormat()
    {
        return m_BackbufferFormat;
    }

    void Dx12Swapchain::Resize(
        const Size2I&   Size,
        EResourceFormat NewFormat)
    {
        if (NewFormat != EResourceFormat::Unknown)
        {
            m_BackbufferFormat = NewFormat;
        }

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