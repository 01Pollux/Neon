#include <EnginePCH.hpp>
#include <Runtime/Types/LoadingScene.hpp>

#include <Runtime/GameEngine.hpp>
#include <Runtime/Renderer.hpp>
#include <Runtime/Pipeline.hpp>

//

#include <Renderer/RG/Passes/Lambda.hpp>

//

#include <Resource/Pack.hpp>
#include <Resource/Manager.hpp>
#include <Resource/Types/Texture.hpp>

#include <RHI/Resource/Views/ShaderResource.hpp>
#include <RHI/Resource/State.hpp>

#include <fstream>
#include <Math/Matrix.hpp>
#include <RHI/Resource/Views/Shader.hpp>

//

namespace Neon::Runtime
{
    LoadingScreenRuntime::LoadingScreenRuntime(
        DefaultGameEngine* Engine)
    {
        EnginePipelineBuilder Builder;

        auto Render         = Builder.NewPhase("Render");
        auto ResourceLoader = Builder.NewPhase("ResourceLoader");

        //

        auto Pipeline = Engine->RegisterInterface<EnginePipeline>(std::move(Builder), 2);
        auto Renderer = Engine->QueryInterface<EngineRenderer>();

        Pipeline->Attach(
            "Render",
            [this, Renderer]
            {
                Renderer->PreRender();
                Renderer->Render();
                Renderer->PostRender();
            });

        SetupRendergraph(Engine);
    }

    struct VsInput
    {
        Vector4 Position;
        Vector2 TexCoord;
    };

    static constexpr size_t BufferSize = sizeof(VsInput) * 6;

    void LoadingScreenRuntime::SetupRendergraph(
        DefaultGameEngine* Engine)
    {
        auto Renderer    = Engine->QueryInterface<EngineRenderer>();
        auto RenderGraph = Renderer->GetRenderGraph();

        auto Builder = RenderGraph->Reset();

        float CurFacePercentage = 0.f;

        auto Pack = Engine->QueryInterface<Asset::IResourceManager>()->GetPack("__neon");

        auto Asset = Pack->Load<Asset::TextureAsset>(
            Asset::AssetHandle::FromString("d0b50bba-f800-4c18-a595-fd5c4b380191"));

        RHI::PendingResource LoadedTexture(
            Renderer->GetSwapchain(),
            Asset->GetImageInfo());

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
                    Desc.Flags.Set(RHI::EShaderCompileFlags::Debug);

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

                    // configure transparency
                    Builder.BlendState.RenderTargets[0].BlendEnable = true;
                    Builder.BlendState.RenderTargets[0].Src         = RHI::BlendTarget::SrcAlpha;
                    Builder.BlendState.RenderTargets[0].Dest        = RHI::BlendTarget::InvSrcAlpha;
                    Builder.BlendState.RenderTargets[0].OpSrc       = RHI::BlendOp::Add;
                    Builder.BlendState.RenderTargets[0].SrcAlpha    = RHI::BlendTarget::One;
                    Builder.BlendState.RenderTargets[0].DestAlpha   = RHI::BlendTarget::Zero;
                    Builder.BlendState.RenderTargets[0].OpAlpha     = RHI::BlendOp::Add;

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
                [LoadedTexture = std::move(LoadedTexture),
                 CurFacePercentage](const RG::GraphStorage& Storage, RHI::ICommandList* CommandList) mutable
                {
                    auto RenderCommandList = dynamic_cast<RHI::IGraphicsCommandList*>(CommandList);
                    auto Swapchain         = Storage.GetSwapchain();

                    auto Texture = LoadedTexture.Access<RHI::ITexture>(RHI::CommandQueueType::Graphics);

                    Swapchain->GetStateManager()->TransitionResource(
                        Texture,
                        RHI::MResourceState::FromEnum(RHI::EResourceState::PixelShaderResource));

                    //

                    auto& RootSignature = Storage.GetRootSignature(RG::ResourceId(STR("Test.RS")));
                    auto& PipelineState = Storage.GetPipelineState(RG::ResourceId(STR("Test.Pipeline")));

                    RenderCommandList->SetRootSignature(RootSignature);
                    RenderCommandList->SetPipelineState(PipelineState);

                    //

                    auto Buffer = Storage.GetResource(RG::ResourceId(STR("Test.Buffer"))).AsUploadBuffer();
                    auto Vertex = Buffer->Map<VsInput>();

                    Vertex[0] = { { -0.40f, -0.40f, 0.0f, 1.0 }, { 0.f, 1.0f } };
                    Vertex[1] = { { -0.40f, +0.40f, 0.0f, 1.0 }, { 0.f, 0.0f } };
                    Vertex[2] = { { +0.40f, -0.40f, 0.0f, 1.0 }, { 1.f, 1.0f } };

                    Vertex[3] = { { -0.40f, +0.40f, 0.0f, 1.0 }, { 0.f, 0.0f } };
                    Vertex[4] = { { +0.40f, +0.40f, 0.0f, 1.0 }, { 1.f, 0.0f } };
                    Vertex[5] = { { +0.40f, -0.40f, 0.0f, 1.0 }, { 1.f, 1.0f } };

                    Buffer->Unmap();

                    //

                    RenderCommandList->SetConstants(0, &CurFacePercentage, 1);

                    auto ResourceView = RHI::Views::ShaderResource(RenderCommandList->GetResourceView());
                    ResourceView.Bind(Texture);

                    RenderCommandList->SetDescriptorTable(
                        1,
                        ResourceView.GetGpuHandle());

                    //

                    RHI::Views::Vertex Vtx;
                    Vtx.Append(Buffer.get(), 0, sizeof(VsInput), sizeof(VsInput) * 6);
                    RenderCommandList->SetVertexBuffer(0, Vtx);

                    RenderCommandList->SetPrimitiveTopology(RHI::PrimitiveTopology::TriangleList);

                    RenderCommandList->Draw(RHI::DrawArgs{
                        .VertexCountPerInstance = 6 });

                    // TODO: delta time
                    CurFacePercentage += 0.03f;
                });

        Builder.Build();
    }
} // namespace Neon::Runtime