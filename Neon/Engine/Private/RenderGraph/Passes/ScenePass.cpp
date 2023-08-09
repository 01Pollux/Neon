#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>

#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/ScenePass.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Camera.hpp>

//

#include <RHI/Swapchain.hpp>
#include <RHI/Commands/List.hpp>

namespace Neon::RG
{
    using namespace Scene;
    using namespace Renderer;

    ScenePass::ScenePass(
        flecs::entity Camera) :
        IRenderPass("ScenePass", PassQueueType::Direct),
        m_Camera(Camera)
    {
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

        auto CameraStorage = UpdateCameraBuffer();
        for (auto& Renderer : m_Renderers)
        {
            Renderer->Render(CameraStorage, RenderCommandList);
        }
    }

    RHI::GpuResourceHandle ScenePass::UpdateCameraBuffer()
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
        return CameraComponent->GraphicsBuffer->GetHandle();
    }
} // namespace Neon::RG