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
    struct VsInput
    {
        Vector2D Position;
    };

    static constexpr size_t BufferSize = sizeof(VsInput) * 4 + sizeof(uint16_t) * 6;

    class TestPass : public IRenderPass
    {
    public:
        TestPass() :
            IRenderPass(PassQueueType::Direct)
        {
        }

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

        void ResolveMaterials(
            MaterialResolver& Resolver) override
        {
        }

        void ResolveResources(
            ResourceResolver& Resolver)
        {
            Resolver.CreateBuffer(
                RG::ResourceId(STR("Test.Buffer")),
                RHI::BufferDesc{ .Size = BufferSize },
                RHI::GraphicsBufferType::Upload);

            Resolver.WriteResource(
                RG::ResourceViewId(STR("FinalImage"), STR("Draw")),
                RHI::RTVDesc{
                    .View      = RHI::RTVDesc::Texture2D{},
                    .ClearType = RHI::ERTClearType::Color,
                    .Format    = Resolver.GetSwapchainFormat(),
                });
        }

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

            auto Buffer = Storage.GetResource(RG::ResourceId(STR("Test.Buffer"))).AsUploadBuffer();

            auto Vertex = Buffer->Map<VsInput>();

            Vertex[0] = { Vector2D(-1.f, -1.f) };
            Vertex[1] = { Vector2D(-1.f, 1.f) };
            Vertex[2] = { Vector2D(1.f, -1.f) };
            Vertex[3] = { Vector2D(1.f, 1.f) };

            auto Input = Buffer->Map<uint16_t>(sizeof(VsInput) * 4);

            Input[0] = 0;
            Input[1] = 1;
            Input[2] = 2;
            Input[3] = 2;
            Input[4] = 1;
            Input[5] = 3;

            Buffer->Unmap();
            Buffer->Unmap();

            //

            Color4 Color = Colors::White;

            static float Time = 0.f;
            Time += 0.03f;
            RenderCommandList->SetConstants(0, &Time, 1);

            RHI::Views::Vertex Vtx;
            Vtx.Append(Buffer.get(), 0, sizeof(VsInput), sizeof(VsInput) * 4);
            RenderCommandList->SetVertexBuffer(0, Vtx);

            RHI::Views::Index Idx(Buffer.get(), sizeof(VsInput) * 4, sizeof(uint16_t) * 6);
            RenderCommandList->SetIndexBuffer(Idx);

            RenderCommandList->SetPrimitiveTopology(RHI::PrimitiveTopology::TriangleList);

            RenderCommandList->SetDynamicResourceView(
                RHI::ICommonCommandList::ViewType::Cbv,
                1,
                Colors::Green.data(),
                sizeof(float) * 4);

            RenderCommandList->Draw(RHI::DrawIndexArgs{
                .IndexCountPerInstance = 6 });
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

        //

        m_RenderGraph = std::make_unique<RG::RenderGraph>(m_Swapchain.get());

        auto Builder = m_RenderGraph->Reset();
        Builder.AppendPass<RG::TestPass>();
        Builder.Build();
    }

    RHI::ISwapchain* Graphics::GetSwapchain() const noexcept
    {
        return m_Swapchain.get();
    }

    void Graphics::PreRender()
    {
        m_Swapchain->PrepareFrame();

        m_RenderGraph->Run();
    }

    void Graphics::PostRender()
    {
        m_Swapchain->Present();
    }
} // namespace Neon::Module