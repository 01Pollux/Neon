#include <EnginePCH.hpp>
#include <Renderer/RG/RG.hpp>
#include <Renderer/RG/Passes/ScenePass.hpp>

#include <glm/gtc/type_ptr.hpp>

//

#include <Scene/Scene.hpp>
#include <Scene/Component/Sprite.hpp>
#include <Scene/Component/Transform.hpp>

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

        //

        // Renderer::MaterialBuilder;
    }

    void ScenePass::ResolveShaders(
        ShaderResolver& Resolver)
    {
    }

    void ScenePass::ResolveRootSignature(
        RootSignatureResolver& Resolver)
    {
    }

    void ScenePass::ResolvePipelineStates(
        PipelineStateResolver& Resolver)
    {
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