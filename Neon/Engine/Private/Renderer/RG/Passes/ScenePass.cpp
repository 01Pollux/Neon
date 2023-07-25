#include <EnginePCH.hpp>
#include <Renderer/RG/RG.hpp>
#include <Renderer/RG/Passes/ScenePass.hpp>

//

#include <Scene/Scene.hpp>
#include <Scene/Component/Camera.hpp>

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

    struct PerFrameData
    {
        alignas(16) Matrix4x4 View;
        alignas(16) Matrix4x4 Projection;
        alignas(16) Matrix4x4 ViewProjection;

        alignas(16) Matrix4x4 ViewInverse;
        alignas(16) Matrix4x4 ProjectionInverse;
        alignas(16) Matrix4x4 ViewProjectionInverse;

        alignas(16) Vector3 CameraPosition;
        alignas(16) Vector3 CameraDirection;
        alignas(16) Vector3 CameraUp;
    };

    ScenePass::ScenePass(
        const GraphStorage&,
        GameScene& Scene,
        Actor      Camera) :
        IRenderPass(PassQueueType::Direct),
        m_Scene(Scene),
        m_Camera(Camera)
    {
        m_SpriteQuery =
            m_Scene->query_builder<
                       Component::Transform,
                       Component::Sprite>()
                // Order by material root signature
                .order_by(
                    +[](flecs::entity_t,
                        const Component::Sprite* LhsSprite,
                        flecs::entity_t,
                        const Component::Sprite* RhsSprite) -> int
                    {
                        return int(LhsSprite->MaterialInstance->GetRootSignature().get() - RhsSprite->MaterialInstance->GetRootSignature().get());
                    })
                // Order by material pipeline state
                .order_by(
                    +[](flecs::entity_t,
                        const Component::Sprite* LhsSprite,
                        flecs::entity_t,
                        const Component::Sprite* RhsSprite) -> int
                    {
                        return int(LhsSprite->MaterialInstance->GetPipelineState().get() - RhsSprite->MaterialInstance->GetPipelineState().get());
                    })
                .build();
        //

        m_SpriteBatch.reset(NEON_NEW SpriteBatch);
        m_CameraBuffer.reset(RHI::IUploadBuffer::Create({ .Size = Math::AlignUp(sizeof(PerFrameData), 256), .Alignment = 256 }));
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

        //
    }

    void ScenePass::Dispatch(
        const GraphStorage&,
        RHI::ICommandList* CommandList)
    {
        auto RenderCommandList = dynamic_cast<RHI::IGraphicsCommandList*>(CommandList);

        if (m_SpriteQuery.is_true())
        {
            UpdateCameraBuffer();

            m_SpriteBatch->SetCameraBuffer(m_CameraBuffer);
            m_SpriteBatch->Begin(RenderCommandList);
            m_SpriteQuery.each(
                [this](const Component::Transform& Transform,
                       const Component::Sprite&    Sprite)
                {
                    m_SpriteBatch->Draw(Transform, Sprite);
                });
            m_SpriteBatch->End();
        }
    }

    void ScenePass::UpdateCameraBuffer()
    {
        auto CameraBuffer    = m_CameraBuffer->Map<PerFrameData>();
        auto CameraComponent = m_Camera.get<Component::Camera>();

        CameraBuffer->View           = glm::transpose(CameraComponent->ViewMatrix(m_Camera));
        CameraBuffer->Projection     = glm::transpose(CameraComponent->ProjectionMatrix());
        CameraBuffer->ViewProjection = CameraBuffer->View * CameraBuffer->Projection;

        CameraBuffer->ViewInverse           = glm::inverse(CameraBuffer->View);
        CameraBuffer->ProjectionInverse     = glm::inverse(CameraBuffer->Projection);
        CameraBuffer->ViewProjectionInverse = glm::inverse(CameraBuffer->ViewProjection);

        // CameraBuffer->CameraPosition  = CameraComponent->GetPosition();
        // CameraBuffer->CameraDirection = CameraComponent->GetDirection();
        // CameraBuffer->CameraUp        = CameraComponent->GetUp();

        m_CameraBuffer->Unmap();
    }
} // namespace Neon::RG