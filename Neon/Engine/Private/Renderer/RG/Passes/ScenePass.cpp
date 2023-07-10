#include <EnginePCH.hpp>
#include <Renderer/RG/RG.hpp>
#include <Renderer/RG/Passes/ScenePass.hpp>

//

#include <Scene/Scene.hpp>
#include <Scene/Component/Sprite.hpp>
#include <Scene/Component/Transform.hpp>

//

#include <RHI/Swapchain.hpp>
#include <RHI/Resource/Resource.hpp>
#include <RHI/Commands/List.hpp>

//

namespace ranges = std::ranges;

namespace Neon::RG
{
    using namespace Scene;
    using namespace Renderer;

    ScenePass::ScenePass(
        const GraphStorage&,
        GameScene& Scene) :
        IRenderPass(PassQueueType::Direct),
        m_Scene(Scene)
    {
        m_SpriteQuery =
            m_Scene->query_builder<
                       Component::Transform,
                       Component::Sprite>()
                .order_by(
                    +[](flecs::entity_t,
                        const Component::Sprite* LhsSprite,
                        flecs::entity_t,
                        const Component::Sprite* RhsSprite) -> int
                    {
                        return int(LhsSprite->MaterialInstance.get() - RhsSprite->MaterialInstance.get());
                    })
                .build();

        //

        m_SpriteBatch.reset(NEON_NEW SpriteBatch);
    }

    void ScenePass::ResolveShaders(
        ShaderResolver&)
    {
    }

    void ScenePass::ResolveRootSignature(
        RootSignatureResolver&)
    {
    }

    void ScenePass::ResolvePipelineStates(
        PipelineStateResolver&)
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
                .Format    = RHI::ISwapchain::Get()->GetFormat(),
            });

        //
    }

    void ScenePass::Dispatch(
        const GraphStorage&,
        RHI::ICommandList* CommandList)
    {
        auto RenderCommandList = dynamic_cast<RHI::IGraphicsCommandList*>(CommandList);

        if (m_SpriteQuery.is_true())
        {
            std::map<IMaterial*, std::vector<flecs::entity>> InstancedSprites;

            m_SpriteQuery.each(
                [this, &InstancedSprites](
                    flecs::entity Entity,
                    Component::Transform&,
                    Component::Sprite& Sprite)
                {
                    InstancedSprites[Sprite.MaterialInstance.get()].emplace_back(Entity);
                });

            m_SpriteBatch->Begin(RenderCommandList);
            for (auto& [Instance, Entities] : InstancedSprites)
            {
                for (flecs::entity Entity : Entities)
                {
                    auto Sprite = Entity.get<Component::Sprite>();
                    m_SpriteBatch->Draw(
                        SpriteBatch::QuadCommand{
                            .Position         = Entity.get<Component::Transform>()->World.GetPosition(),
                            .Size             = Sprite->Size,
                            .Color            = Sprite->ModulationColor,
                            .MaterialInstance = Instance });
                }
            }
            m_SpriteBatch->End();
        }
    }
} // namespace Neon::RG