#include <EnginePCH.hpp>
#include <Renderer/RG/RG.hpp>
#include <Renderer/RG/Passes/ScenePass.hpp>
#include <Scene/Component/Camera.hpp>
#include <Runtime/GameEngine.hpp>

//

#include <RHI/Swapchain.hpp>
#include <RHI/Commands/List.hpp>

//

namespace ranges = std::ranges;

namespace Neon::RG
{
    using namespace Scene;
    using namespace Renderer;

    ScenePass::ScenePass(
        const GraphStorage&,
        GameScene& Scene,
        Actor      Camera) :
        IRenderPass(PassQueueType::Direct),
        m_Scene(Scene),
        m_Camera(Camera)
    {
        m_SpriteQuery =
            m_Scene.GetEntityWorld()
                ->query_builder<Component::Transform, Component::Sprite>()
                .term<Component::Sprite>()
                .in()
                .term<Component::Transform>()
                .in()
                // Order by material root signature & pipeline state
                .order_by(
                    +[](flecs::entity_t,
                        const Component::Sprite* LhsSprite,
                        flecs::entity_t,
                        const Component::Sprite* RhsSprite) -> int
                    {
                        // Order by material root signature and pipeline state, this is like qsort compare function, pipeline state and root signature!
                        return int((intptr_t(LhsSprite->MaterialInstance->GetRootSignature().get()) - intptr_t(RhsSprite->MaterialInstance->GetRootSignature().get()))) * 1000 +
                               int((intptr_t(LhsSprite->MaterialInstance->GetPipelineState().get()) - intptr_t(RhsSprite->MaterialInstance->GetPipelineState().get())));
                    })
                .build();
        //
    }

    void ScenePass::ResolveResources(
        ResourceResolver& Resolver)
    {
        Resolver.WriteResource(
            RG::ResourceViewId(STR("OutputImage"), STR("ScenePass")),
            RHI::RTVDesc{
                .View      = RHI::RTVDesc::Texture2D{},
                .ClearType = RHI::ERTClearType::Color,
                .Format    = RHI::ISwapchain::Get()->GetFormat(),
            });
    }

    void ScenePass::Dispatch(
        const GraphStorage& Storage,
        RHI::ICommandList*  CommandList)
    {
        auto RenderCommandList = dynamic_cast<RHI::IGraphicsCommandList*>(CommandList);

        if (m_SpriteQuery.is_true())
        {
            auto CameraStorage = UpdateCameraBuffer();

            m_SpriteBatch.SetCameraBuffer(CameraStorage);
            m_SpriteBatch.Begin(RenderCommandList);
            m_SpriteQuery.each(
                [this](const Component::Transform& Transform,
                       const Component::Sprite&    Sprite)
                {
                    m_SpriteBatch.Draw(Transform, Sprite);
                });
            m_SpriteBatch.End();

            //
        }
    }

    Ptr<RHI::IUploadBuffer> ScenePass::UpdateCameraBuffer()
    {
        auto CameraComponent = m_Camera.get<Component::Camera>();
        auto CameraTransform = m_Camera.get<Component::Transform>();
        auto CameraBuffer    = CameraComponent->GraphicsBuffer->Map<Component::CameraFrameData>();

        CameraBuffer->World = glm::transpose(CameraTransform->World.ToMat4x4());

        CameraBuffer->View           = glm::transpose(CameraComponent->ViewMatrix(m_Camera));
        CameraBuffer->Projection     = glm::transpose(CameraComponent->ProjectionMatrix());
        CameraBuffer->ViewProjection = CameraBuffer->View * CameraBuffer->Projection;

        CameraBuffer->ViewInverse           = glm::inverse(CameraBuffer->View);
        CameraBuffer->ProjectionInverse     = glm::inverse(CameraBuffer->Projection);
        CameraBuffer->ViewProjectionInverse = glm::inverse(CameraBuffer->ViewProjection);

        CameraBuffer->EngineTime = float(Runtime::DefaultGameEngine::Get()->GetEngineTime());
        CameraBuffer->GameTime   = float(Runtime::DefaultGameEngine::Get()->GetGameTime());
        CameraBuffer->DeltaTime  = float(Runtime::DefaultGameEngine::Get()->GetDeltaTime());

        CameraComponent->GraphicsBuffer->Unmap();
        return CameraComponent->GraphicsBuffer;
    }
} // namespace Neon::RG