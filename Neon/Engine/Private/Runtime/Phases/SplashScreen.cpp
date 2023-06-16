#include <EnginePCH.hpp>
#include <Runtime/Phases/SplashScreen.hpp>

#include <Runtime/Pipeline.hpp>
#include <Runtime/GameEngine.hpp>

#include <Module/Graphics.hpp>
#include <Renderer/RG/Passes/Lambda.hpp>

//

#include <RHI/Resource/Views/ShaderResource.hpp>

#include <fstream>
#include <Math/Matrix.hpp>
#include <RHI/Resource/Views/Shader.hpp>

namespace Neon::Runtime::Phases
{
    struct VsInput
    {
        Vector4D Position;
        Vector4D Color;
    };

    static constexpr size_t BufferSize = sizeof(VsInput) * 4;

    static constexpr const char* SplashScreenTag = "SplashScreen";

    void SplashScreen::Build(
        EnginePipelineBuilder& Builder)
    {
        //
    }

    void SplashScreen::Bind(
        DefaultGameEngine* Engine)
    {
        auto& Pipeline = Engine->GetPipeline();

        auto Graphics    = Engine->GetGraphicsModule();
        auto RenderGraph = Graphics->GetRenderGraph();

        auto Builder = RenderGraph->Reset();

        Ptr<RHI::ITexture> Texture;

        std::vector<uint8_t> Data(256 * 256 * 4);
        for (size_t i = 0; i < Data.size(); i += 4)
        {
            Data[i + 0] = 255;
            Data[i + 1] = 0;
            Data[i + 2] = 0;
            Data[i + 3] = 255;
        }

        RHI::SubresourceDesc Subresources{
            .Data       = Data.data(),
            .RowPitch   = 256 * 4,
            .SlicePitch = Subresources.RowPitch * 256
        };

        uint64_t CopyId;

        Texture.reset(RHI::ITexture::Create(
            Graphics->GetSwapchain(),
            RHI::ResourceDesc::Tex2D(RHI::EResourceFormat::R8G8B8A8_UNorm, 256, 256, 1, 1),
            { &Subresources, 1 },
            CopyId));

        Builder.AppendPass<RG::LambdaPass>(RG::PassQueueType::Direct)
            .SetShaderResolver(
                [](auto& Resolver)
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
                })
            .SetRootSignatureResolver(
                [](auto& Resolver)
                {
                    RHI::StaticSamplerDesc Sampler;

                    Sampler.Filter         = RHI::ESamplerFilter::MinMagMipPoint;
                    Sampler.RegisterSpace  = 0;
                    Sampler.ShaderRegister = 0;
                    Sampler.Visibility     = RHI::ShaderVisibility::Pixel;

                    Resolver.Load(
                        RG::ResourceId(STR("Test.RS")),
                        RHI::RootSignatureBuilder()
                            .Add32BitConstants<float>(0, 0)
                            .Add32BitConstants<Matrix4x4>(0, 1)
                            .AddConstantBufferView(1, 0)
                            .AddDescriptorTable(
                                RHI::RootDescriptorTable()
                                    .AddSrvRange(0, 0, 1))
                            .AddSampler(Sampler)
                            .SetFlags(RHI::ERootSignatureBuilderFlags::AllowInputLayout));
                })
            .SetPipelineStateResolver(
                [](auto& Resolver)
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
                })
            .SetResourceResolver(
                [](auto& Resolver)
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
                })
            .SetDispatcher(
                [Texture, CopyId = std::optional{ CopyId }](const RG::GraphStorage& Storage, RHI::ICommandList* CommandList)
                {
                    auto RenderCommandList = dynamic_cast<RHI::IGraphicsCommandList*>(CommandList);

                    if (CopyId)
                    {
                        auto GraphicsQueue = Storage.GetSwapchain()->GetQueue(RHI::CommandQueueType::Graphics);
                        Storage.GetSwapchain()->WaitForCopy(
                            GraphicsQueue,
                            *CopyId);
                    }

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

                    auto ResourceView = RenderCommandList->GetResourceView();
                    auto Srv          = static_cast<RHI::Views::ShaderResource&>(ResourceView);
                    Srv.Bind(Texture.get());

                    RenderCommandList->SetDescriptorTable(
                        3,
                        ResourceView.GetGpuHandle());

                    RenderCommandList->Draw(RHI::DrawArgs{
                        .VertexCountPerInstance = 3 });
                });

        Builder.Build();
    }
} // namespace Neon::Runtime::Phases