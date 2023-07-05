#include <EnginePCH.hpp>
#include <Renderer/RG/RG.hpp>
#include <Renderer/RG/Passes/ScenePass.hpp>

//

#include <Scene/Scene.hpp>
#include <Scene/Component/Sprite.hpp>
#include <Scene/Component/Transform.hpp>
#include <Renderer/Material/Builder.hpp>

//

#include <RHI/Resource/Resource.hpp>
#include <RHI/Commands/List.hpp>

//

namespace Neon::RG
{
    using namespace Scene;

    ScenePass::ScenePass(
        const GraphStorage& Storage,
        GameScene&          Scene) :
        IRenderPass(PassQueueType::Direct),
        m_Scene(Scene)
    {
        m_SpriteQuery = m_Scene->query_builder<
                                   Component::Transform,
                                   Component::Sprite>()
                            .build();

        //

        Renderer::RenderMaterialBuilder Builder;

        Builder.VertexShader(Asset::ShaderModuleId(1));
        Builder.PixelShader(Asset::ShaderModuleId(1));

        auto Material = std::make_shared<Renderer::Material>(Builder);

        m_SpriteBatch.reset(
            NEON_NEW Renderer::SpriteBatch(
                std::move(Material),
                Storage.GetSwapchain()));
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
        return;
        auto RenderCommandList = dynamic_cast<RHI::IGraphicsCommandList*>(CommandList);

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