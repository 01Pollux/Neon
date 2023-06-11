#include <EnginePCH.hpp>
#include <Module/Graphics.hpp>
#include <Module/Window.hpp>

#include <Renderer/RG/RG.hpp>

#include <fstream>

#include <RHI/PipelineState.hpp>
#include <RHI/Resource/State.hpp>
#include <RHI/RootSignature.hpp>
#include <RHI/Resource/Views/Shader.hpp>

namespace Neon::RG
{
    class TestPass : public IRenderPass
    {
    public:
        TestPass() :
            IRenderPass(PassQueueType::Direct)
        {
        }

        /// <summary>
        /// Called when the render pass wans to load shaders.
        /// </summary>
        void ResolveShaders(
            ShaderResolver& Resolver) override
        {
            std::stringstream Stream;
            std::ifstream     File(L"D:\\Dev\\Shader.hlsl");

            Stream << File.rdbuf();
            auto Text = Stream.str();

            RHI::ShaderCompileDesc Desc{
                .Stage      = RHI::ShaderStage::Vertex,
                .SourceCode = Text,
                .EntryPoint = STR("VSMain")
            };

            Resolver.Load(
                RG::ResourceId(STR("Test.VS")),
                Desc);

            Desc.Stage      = RHI::ShaderStage::Pixel;
            Desc.EntryPoint = STR("PSMain");

            Resolver.Load(
                RG::ResourceId(STR("Test.PS")),
                Desc);
        }

        /// <summary>
        /// Called when the render pass wans to load shaders.
        /// </summary>
        void ResolveRootSignature(
            RootSignatureResolver& Resolver) override
        {
            Resolver.Load(
                RG::ResourceId(STR("Test.RS")),
                RHI::RootSignatureBuilder()
                    .Add32BitConstants<float>(0, 0)
                    .AddConstantBufferView(1, 0)
                    .SetFlags(RHI::ERootSignatureBuilderFlags::AllowInputLayout));
        }

        /// <summary>
        /// Called when the render pass wants to create pipelines.
        /// </summary>
        void ResolvePipelines(
            PipelineStateResolver& Resolver) override
        {
            auto& RootSig = Resolver.GetRootSignature(RG::ResourceId(STR("Test.RS")));
            auto& VS      = Resolver.GetShader(RG::ResourceId(STR("Test.VS")));
            auto& PS      = Resolver.GetShader(RG::ResourceId(STR("Test.PS")));

            RHI::PipelineStateBuilder<false> Builder{
                .RootSignature     = RootSig.get(),
                .VertexShader      = VS.get(),
                .PixelShader       = PS.get(),
                .DepthStencilState = { .DepthEnable = false },
                .PrimitiveTopology = RHI::PipelineStateBuilder<false>::Toplogy::Triangle,
                .RTFormats         = { RHI::EResourceFormat::R8G8B8A8_UNorm },
            };

            VS->CreateInputLayout(Builder.InputLayout);

            Resolver.Load(
                RG::ResourceId(STR("Test.Pipeline")),
                Builder);
        }

        /// <summary>
        /// Called when the render pass wants to load materials.
        /// </summary>
        void ResolveMaterials(
            MaterialResolver& Resolver) override
        {
        }

        /// <summary>
        /// Called when the render pass wants to resolve the dependencies of resources.
        /// </summary>
        void ResolveResources(
            ResourceResolver& Resolver)
        {
        }

        /// <summary>
        /// Called to check if the pass should implements its own viewports
        /// </summary>
        bool OverrideViewport(
            const GraphStorage&        Storage,
            RHI::IGraphicsCommandList* CommandList) override
        {
            return false;
        }

        /// <summary>
        /// Called when the render pass wants to dispatch.
        /// </summary>
        void Dispatch(
            const GraphStorage& Storage,
            RHI::ICommandList*  CommandList) override
        {
            auto RenderCommandList = dynamic_cast<RHI::IGraphicsCommandList*>(CommandList);

            //

            auto& RootSignature = Storage.GetRootSignature(RG::ResourceId(STR("Test.RS")));
            auto& PipelineState = Storage.GetPipelineState(RG::ResourceId(STR("Test.Pipeline")));

            RenderCommandList->SetRootSignature(RootSignature);
            RenderCommandList->SetPipelineState(PipelineState);

            //

            struct VsInput
            {
                Vector2D Position;
            };

            Ptr<RHI::IUploadBuffer> m_Buffer;
            m_Buffer.reset(RHI::IUploadBuffer::Create(
                Storage.GetSwapchain(),
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

            auto Rtv          = Storage.GetSwapchain()->GetBackBufferView();
            auto Backbuffer   = Storage.GetSwapchain()->GetBackBuffer();
            auto StateManager = Storage.GetSwapchain()->GetStateManager();

            StateManager->TransitionResource(Backbuffer, RHI::MResourceState::FromEnum(RHI::EResourceState::RenderTarget));
            StateManager->FlushBarriers(CommandList);

            Color4 Color = Colors::White;

            RenderCommandList->ClearRtv(Rtv, Color);
            RenderCommandList->SetRenderTargets(Rtv, 1);

            static float Time = 0.f;
            Time += 0.03f;
            RenderCommandList->SetConstants(0, &Time, 1);

            RHI::Views::Vertex Vtx;
            Vtx.Append(m_Buffer.get(), 0, sizeof(VsInput), sizeof(VsInput) * 4);
            RenderCommandList->SetVertexBuffer(0, Vtx);

            RHI::Views::Index Idx(m_Buffer.get(), sizeof(VsInput) * 4, sizeof(uint16_t) * 6);
            RenderCommandList->SetIndexBuffer(Idx);

            RenderCommandList->SetPrimitiveTopology(RHI::PrimitiveTopology::TriangleList);

            RenderCommandList->SetViewport(ViewportF{
                .Width  = 1280.f,
                .Height = 720.f });

            RenderCommandList->SetScissorRect(
                RectF(Vector2D::Zero, { 1280.f, 720.f }));

            RenderCommandList->SetDynamicResourceView(
                RHI::ICommonCommandList::ViewType::Cbv,
                1,
                Colors::Green.data(),
                sizeof(float) * 4);

            RenderCommandList->Draw(RHI::DrawIndexArgs{
                .IndexCountPerInstance = 6 });

            StateManager->TransitionResource(Backbuffer, RHI::MResourceState_Common);
            StateManager->FlushBarriers(CommandList);
        }
    };
} // namespace Neon::RG

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
    }

    RHI::ISwapchain* Graphics::GetSwapchain() const noexcept
    {
        return m_Swapchain.get();
    }

    void Graphics::PreRender()
    {
        m_Swapchain->PrepareFrame();

        RG::RenderGraph Graph(m_Swapchain.get());

        auto Builder = Graph.Reset();
        Builder.AppendPass<RG::TestPass>();
        Builder.Build();

        Graph.Run();
    }

    void Graphics::PostRender()
    {
        m_Swapchain->Present();
    }
} // namespace Neon::Module