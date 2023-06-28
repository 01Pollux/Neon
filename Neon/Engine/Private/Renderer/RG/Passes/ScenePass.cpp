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
                        .AddSrvRange(0, 0, 10'000,
                                     RHI::MRootDescriptorTableFlags::FromEnum(RHI::ERootDescriptorTableFlags::Descriptor_Volatile)))
                .AddSampler(Sampler)
                .SetFlags(RHI::ERootSignatureBuilderFlags::AllowInputLayout));
    }

    void ScenePass::ResolvePipelineStates(
        PipelineStateResolver& Resolver)
    {
        auto& RootSig = Resolver.GetRootSignature(RG::ResourceId(STR("Sprite.RS")));

        auto& VS = Resolver.GetShader(RG::ResourceId(STR("Sprite.VS")));
        auto& PS = Resolver.GetShader(RG::ResourceId(STR("Sprite.PS")));

        RHI::PipelineStateBuilderG Builder{
            .RootSignature  = RootSig.get(),
            .VertexShader   = VS.get(),
            .PixelShader    = PS.get(),
            .Rasterizer     = { .CullMode = RHI::CullMode::None },
            .DepthStencil   = { .DepthEnable = false },
            .UseVertexInput = true,
            .Topology       = RHI::PipelineStateBuilderG::PrimitiveTopology::Triangle,
            .RTFormats      = { RHI::EResourceFormat::R8G8B8A8_UNorm },
        };

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
    }

    void ScenePass::Dispatch(
        const GraphStorage& Storage,
        RHI::ICommandList*  CommandList)
    {
        if (!m_SpriteQuery.is_true())
        {
            return;
        }

        if (!m_SpriteBatch)
        {
            m_SpriteBatch = std::make_unique<Renderer::SpriteBatch>(
                Renderer::SpriteBatch::CompiledPipelineState{
                    .QuadPipelineState = Storage.GetPipelineState(RG::ResourceId(STR("Sprite.Pipeline"))),
                    .QuadRootSignature = Storage.GetRootSignature(RG::ResourceId(STR("Sprite.RS"))),
                },
                Storage.GetSwapchain());
        }

        auto RenderCommandList = dynamic_cast<RHI::IGraphicsCommandList*>(CommandList);

        //

        /*

        m_SpriteBatcher->Begin();

        m_SpriteQuery->each(
            [this](const Component::Transform& Transform, const Component::Sprite& Sprite)
            {
                m_SpriteBatcher->Draw(
                    QuadCommand{
                        .Position = Transform.World.GetPosition(),
                        .Texture  = Sprite.Texture,
                        .Rect     = Sprite.TextureRect,
                        .Color    = Sprite.ModulationColor,
                        .Scale    = Sprite.Scale,
                        .Rotation = Sprite.Rotation });
            });

        */

        if (m_SpriteQuery.is_true())
        {
            m_SpriteBatch->Begin(RenderCommandList);
            m_SpriteQuery.each(
                [this](const Component::Transform& Transform, const Component::Sprite& Sprite)
                {
                    m_SpriteBatch->Draw(
                        Renderer::SpriteBatch::QuadCommand{
                            .Position = Transform.World.GetPosition(),
                            .Size     = Sprite.Size,
                            .Color    = Sprite.ModulationColor }

                    );
                });
            m_SpriteBatch->End();
        }
    }
} // namespace Neon::RG