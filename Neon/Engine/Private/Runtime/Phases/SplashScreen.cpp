#include <EnginePCH.hpp>
#include <Runtime/Phases/SplashScreen.hpp>

#include <Runtime/Pipeline.hpp>
#include <Runtime/GameEngine.hpp>

#include <Module/Graphics.hpp>
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
        auto Graphics    = Engine->GetGraphicsModule();
        auto RenderGraph = Graphics->GetRenderGraph();

        auto Builder = RenderGraph->Reset();

        float CurFacePercentage = 0.f;

        auto Pack = Engine->GetResourceManager()->GetPack("__neon")->Load<Asset::TextureAsset>(
            Asset::AssetHandle::FromString("d0b50bba-f800-4c18-a595-fd5c4b380191"));

        RHI::PendingResource LoadedTexture(
            Graphics->GetSwapchain(),
            Pack->GetImageInfo());

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

                    Builder.BlendState.RenderTargets[0] = {
                        .BlendEnable = true
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

                    Vertex[0] = { { +0.00f, +0.50f, 0.0f, 1.0 }, { 1.0f, 0.0f, 0.0f, 1.0f } }; // top
                    Vertex[1] = { { +0.40f, -0.25f, 0.0f, 1.0 }, { 0.0f, 0.0f, 1.0f, 1.0f } }; // right
                    Vertex[2] = { { -0.40f, -0.25f, 0.0f, 1.0 }, { 0.0f, 1.0f, 0.0f, 1.0f } }; // left

                    Buffer->Unmap();

                    //

                    RenderCommandList->SetConstants(0, &CurFacePercentage, 0);

                    auto ResourceView = RHI::Views::ShaderResource(RenderCommandList->GetResourceView());
                    ResourceView.Bind(Texture);

                    RenderCommandList->SetDescriptorTable(
                        1,
                        ResourceView.GetGpuHandle());

                    //

                    RHI::Views::Vertex Vtx;
                    Vtx.Append(Buffer.get(), 0, sizeof(VsInput), sizeof(VsInput) * 3);
                    RenderCommandList->SetVertexBuffer(0, Vtx);

                    RenderCommandList->SetPrimitiveTopology(RHI::PrimitiveTopology::TriangleList);

                    RenderCommandList->Draw(RHI::DrawArgs{
                        .VertexCountPerInstance = 3 });

                    // TODO: delta time
                    CurFacePercentage += 0.01f;
                });

        Builder.Build();
    }
} // namespace Neon::Runtime::Phases