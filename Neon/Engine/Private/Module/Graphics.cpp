#include <EnginePCH.hpp>
#include <Module/Graphics.hpp>
#include <Module/Window.hpp>

#include <Renderer/RG/RG.hpp>

#include <fstream>
#include <Math/Matrix.hpp>

#include <RHI/PipelineState.hpp>
#include <RHI/Resource/State.hpp>
#include <RHI/RootSignature.hpp>
#include <RHI/Resource/Views/Shader.hpp>

namespace Neon::RG
{
    struct VsInput
    {
        Vector4D Position;
        Vector4D Color;
    };

    static constexpr size_t BufferSize = sizeof(VsInput) * 4;

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
                    .Add32BitConstants<Matrix4x4>(0, 1)
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
                .RasterizerState   = { .CullMode = RHI::CullMode::None },
                .DepthStencilState = { .DepthEnable = false },
                .PrimitiveTopology = RHI::PipelineStateBuilder<false>::Toplogy::Triangle,
                .RTFormats         = { RHI::EResourceFormat::R8G8B8A8_UNorm },
            };

            VS->CreateInputLayout(Builder.InputLayout);

            Resolver.Load(
                RG::ResourceId(STR("Test.Pipeline")),
                Builder);
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

            Vertex[0] = { { +0.00f, +0.50f, 0.0f, 1.0 }, { 1.0f, 0.0f, 0.0f, 1.0f } }; // top
            Vertex[1] = { { +0.40f, -0.25f, 0.0f, 1.0 }, { 0.0f, 0.0f, 1.0f, 1.0f } }; // right
            Vertex[2] = { { -0.40f, -0.25f, 0.0f, 1.0 }, { 0.0f, 1.0f, 0.0f, 1.0f } }; // left

            Buffer->Unmap();

            //

            Color4 Color = Colors::White;

            static float Time = 0.f;
            Time += 0.03f;
            RenderCommandList->SetConstants(0, &Time, 1);

            //

            auto Size = Storage.GetSwapchain()->GetWindow()->GetSize();

            auto View = Matrix4x4::LookAt(
                Vector3D(0, 0, -1),
                Vector3D::Zero,
                Vector3D::Up);

            auto Proj = Matrix4x4::PerspectiveFov(
                DegreesToRadians(65.f),
                float(Size.Width()) / Size.Height(),
                0.1f,
                100.f);

            auto World = Matrix4x4::RotationZ(Time);

            auto WVP = (World * View * Proj);
            // auto WVP = Matrix4x4::Identity;
            RenderCommandList->SetConstants(1, &WVP, sizeof(WVP) / 4);

            //

            RHI::Views::Vertex Vtx;
            Vtx.Append(Buffer.get(), 0, sizeof(VsInput), sizeof(VsInput) * 3);
            RenderCommandList->SetVertexBuffer(0, Vtx);

            RenderCommandList->SetPrimitiveTopology(RHI::PrimitiveTopology::TriangleList);

            RenderCommandList->SetDynamicResourceView(
                RHI::ICommonCommandList::ViewType::Cbv,
                2,
                Colors::Green.data(),
                sizeof(float) * 4);

            RenderCommandList->Draw(RHI::DrawArgs{
                .VertexCountPerInstance = 3 });
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
    }

    RHI::ISwapchain* Graphics::GetSwapchain() noexcept
    {
        return m_Swapchain.get();
    }

    RG::RenderGraph* Graphics::GetRenderGraph() noexcept
    {
        return m_RenderGraph.get();
    }

    void Graphics::PreRender()
    {
        m_Swapchain->PrepareFrame();
    }

    void Graphics::Render()
    {
        m_RenderGraph->Run();
    }

    void Graphics::PostRender()
    {
        m_Swapchain->Present();
    }
} // namespace Neon::Module