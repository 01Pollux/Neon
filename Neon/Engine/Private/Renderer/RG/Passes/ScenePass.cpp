#include <EnginePCH.hpp>
#include <Renderer/RG/RG.hpp>
#include <Renderer/RG/Passes/ScenePass.hpp>

#include <glm/gtc/type_ptr.hpp>

//

#include <RHI/Resource/Views/Shader.hpp>

//

#include <Scene/Scene.hpp>
#include <Scene/Component/Sprite.hpp>
#include <Scene/Component/Transform.hpp>

//

#include <fstream>

//

#include <RHI/Resource/Resource.hpp>
#include <RHI/Commands/List.hpp>

//

namespace Neon::RG
{
    struct VSInput
    {
        Vector4 Position;
        Color4  Color = Colors::Red;
        Vector2 TexCoord;
    };

    using namespace Scene;

    ScenePass::ScenePass(
        GameScene& Scene) :
        IRenderPass(PassQueueType::Direct),
        m_Scene(Scene)
    {
        m_SpriteQuery = m_Scene->query_builder<
                                   Component::Transform,
                                   Component::Sprite>()
                            .build();
    }

    void ScenePass::ResolveShaders(
        ShaderResolver& Resolver)
    {
        std::stringstream Stream;
        std::ifstream     File(L"D:\\Dev\\Sprite.hlsl");

        Stream << File.rdbuf();
        auto Text = Stream.str();

        RHI::ShaderCompileDesc Desc{
            .Stage      = RHI::ShaderStage::Vertex,
            .SourceCode = Text,
            .EntryPoint = STR("VSMain")
        };
        Desc.Flags.Set(RHI::EShaderCompileFlags::Debug);

        Resolver.Load(
            RG::ResourceId(STR("Sprite.VS")),
            Desc);

        Desc.Stage      = RHI::ShaderStage::Pixel;
        Desc.EntryPoint = STR("PSMain");

        Resolver.Load(
            RG::ResourceId(STR("Sprite.PS")),
            Desc);
    }

    void ScenePass::ResolveRootSignature(
        RootSignatureResolver& Resolver)
    {
        RHI::StaticSamplerDesc Sampler;

        Sampler.Filter         = RHI::ESamplerFilter::MinMagMipPoint;
        Sampler.RegisterSpace  = 0;
        Sampler.ShaderRegister = 0;
        Sampler.Visibility     = RHI::ShaderVisibility::Pixel;

        Resolver.Load(
            RG::ResourceId(STR("Sprite.RS")),
            RHI::RootSignatureBuilder()
                .AddDescriptorTable(
                    RHI::RootDescriptorTable()
                        .AddSrvRange(0, 0, 1))
                .AddSampler(Sampler)
                .SetFlags(RHI::ERootSignatureBuilderFlags::AllowInputLayout));
    }

    void ScenePass::ResolvePipelineStates(
        PipelineStateResolver& Resolver)
    {
        auto& RootSig = Resolver.GetRootSignature(RG::ResourceId(STR("Sprite.RS")));

        auto& VS = Resolver.GetShader(RG::ResourceId(STR("Sprite.VS")));
        auto& PS = Resolver.GetShader(RG::ResourceId(STR("Sprite.PS")));

        RHI::PipelineStateBuilder<false> Builder{
            .RootSignature   = RootSig.get(),
            .VertexShader    = VS.get(),
            .PixelShader     = PS.get(),
            .RasterizerState = { .CullMode = RHI::CullMode::None },
            .DepthStencilState{ .DepthEnable = false },
            .PrimitiveTopology = RHI::PipelineStateBuilder<false>::Toplogy::Triangle,
            .RTFormats         = { RHI::EResourceFormat::R8G8B8A8_UNorm },
        };

        VS->CreateInputLayout(Builder.InputLayout);

        Resolver.Load(
            RG::ResourceId(STR("Sprite.Pipeline")),
            Builder);
    }

    void ScenePass::ResolveResources(
        ResourceResolver& Resolver)
    {
        Resolver.WriteResource(
            RG::ResourceViewId(STR("FinalImage"), STR("ScenePass")),
            RHI::RTVDesc{
                .View      = RHI::RTVDesc::Texture2D{},
                .ClearType = RHI::ERTClearType::Color,
                .Format    = Resolver.GetSwapchainFormat(),
            });

        //

        Resolver.CreateBuffer(
            RG::ResourceId(STR("SpriteVertexBuffer")),
            RHI::BufferDesc{
                .Size = 1024 * 4 * sizeof(VSInput) },
            RHI::GraphicsBufferType::Upload);

        Resolver.CreateBuffer(
            RG::ResourceId(STR("SpriteIndexBuffer")),
            RHI::BufferDesc{
                .Size = 1024 * 4 * sizeof(uint16_t) },
            RHI::GraphicsBufferType::Upload);
    }

    void ScenePass::Dispatch(
        const GraphStorage& Storage,
        RHI::ICommandList*  CommandList)
    {
        if (!m_SpriteQuery.is_true())
        {
            return;
        }

        auto RenderCommandList = dynamic_cast<RHI::IGraphicsCommandList*>(CommandList);

        //

        auto& Pipeline      = Storage.GetPipelineState(RG::ResourceId(STR("Sprite.Pipeline")));
        auto& RootSignature = Storage.GetRootSignature(RG::ResourceId(STR("Sprite.RS")));

        RenderCommandList->SetRootSignature(RootSignature);
        RenderCommandList->SetPipelineState(Pipeline);

        //

        //

        auto VertexBuffer = Storage.GetResource(RG::ResourceId(STR("SpriteVertexBuffer"))).AsUploadBuffer();
        auto IndexBuffer  = Storage.GetResource(RG::ResourceId(STR("SpriteIndexBuffer"))).AsUploadBuffer();

        size_t DrawIndex = 0;

        auto DrawToBuffer =
            [VertexBuffer, IndexBuffer, DrawIndex](
                const Component::Transform& Transform,
                const Component::Sprite&    Sprite) mutable
        {
            // cast glm::vec3 to glm::vec4 and set w to 1 for matrix multiplication
            auto World = Vector4(Transform.World.GetPosition(), 1.0f);

            //

            auto Vertex = VertexBuffer->Map<VSInput>(DrawIndex * sizeof(VSInput));

            Vertex[0].Position = World + Vector4(Vector2(Sprite.Scale.x * -0.5f, Sprite.Scale.y * +0.5f), 0.f, 0.f);
            Vertex[1].Position = World + Vector4(Vector2(Sprite.Scale.x * +0.5f, Sprite.Scale.y * +0.5f), 0.f, 0.f);
            Vertex[2].Position = World + Vector4(Vector2(Sprite.Scale.x * +0.5f, Sprite.Scale.y * -0.5f), 0.f, 0.f);
            Vertex[3].Position = World + Vector4(Vector2(Sprite.Scale.x * -0.5f, Sprite.Scale.y * -0.5f), 0.f, 0.f);

            Vertex[0].TexCoord = Sprite.TextureRect.TopLeft();
            Vertex[1].TexCoord = Sprite.TextureRect.TopRight();
            Vertex[2].TexCoord = Sprite.TextureRect.BottomRight();
            Vertex[3].TexCoord = Sprite.TextureRect.BottomLeft();

            Vertex[0].Color = Sprite.ModulationColor;
            Vertex[1].Color = Sprite.ModulationColor;
            Vertex[2].Color = Sprite.ModulationColor;
            Vertex[3].Color = Sprite.ModulationColor;

            //

            auto Index = IndexBuffer->Map<uint16_t>(DrawIndex * sizeof(uint16_t));

            Index[0] = 0;
            Index[1] = 1;
            Index[2] = 2;

            Index[3] = 0;
            Index[4] = 2;
            Index[5] = 3;

            //

            DrawIndex++;

            IndexBuffer->Unmap();
            VertexBuffer->Unmap();
        };

        //

        if (m_SpriteQuery.is_true())
        {
            m_SpriteQuery.each(DrawToBuffer);

            RHI::Views::Vertex Vtx;
            Vtx.Append(VertexBuffer.get(), 0, sizeof(VSInput), sizeof(VSInput) * 4);
            RenderCommandList->SetVertexBuffer(0, Vtx);

            RHI::Views::Index Idx(IndexBuffer.get(), 0, sizeof(uint16_t) * 6);
            RenderCommandList->SetIndexBuffer(Idx);

            RenderCommandList->SetPrimitiveTopology(RHI::PrimitiveTopology::TriangleList);

            RenderCommandList->Draw(RHI::DrawIndexArgs{
                .IndexCountPerInstance = 6 });
        }
    }
} // namespace Neon::RG