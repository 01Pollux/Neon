#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>

#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/ScenePass.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Camera.hpp>

namespace Neon::RG
{
    using namespace Scene;
    using namespace Renderer;

    ScenePass::ScenePass(
        flecs::entity Camera) :
        IRenderPass("ScenePass", PassQueueType::Direct),
        m_Camera(Camera)
    {
    }

    void ScenePass::ResolveResources(
        ResourceResolver& Resolver)
    {
        /*  auto ResourceDesc = RHI::ResourceDesc::Tex2D(
              RHI::EResourceFormat::R8G8B8A8_UNorm,
              0, 0, 1, 1);

          auto DepthDesc = RHI::ResourceDesc::Tex2D(
              RHI::EResourceFormat::R32_Typeless,
              0, 0, 1, 1);

          std::array Resources{
              std::pair{ ResourceId(STR("GBufferAlbedo")), &ResourceDesc },
              std::pair{ ResourceId(STR("GBufferNormal")), &ResourceDesc },
              std::pair{ ResourceId(STR("GBufferEmissive")), &ResourceDesc }
          };

          for (auto& [Resource, Desc] : Resources)
          {
              Resolver.CreateWindowTexture(ResourceId(STR("GBufferAlbedo")), *Desc);

              if (Desc == &ResourceDesc) [[likely]]
              {
                  Resolver.WriteResource(
                      Resource.CreateView(STR("Main")),
                      RHI::RTVDesc{
                          .View      = RHI::RTVDesc::Texture2D{},
                          .ClearType = RHI::ERTClearType::Color,
                          .Format    = RHI::EResourceFormat::R8G8B8A8_UNorm,
                      });
              }
              else
              {
                  Resolver.WriteResource(
                      Resource.CreateView(STR("Main")),
                      RHI::DSVDesc{
                          .View       = RHI::DSVDesc::Texture2D{},
                          .ForceDepth = 1.f,
                          .ClearType  = RHI::EDSClearType::Depth,
                      });
              }
          }*/

        Resolver.WriteResource(
            RG::ResourceViewId(STR("OutputImage"), STR("ScenePass")),
            RHI::RTVDesc{
                .View      = RHI::RTVDesc::Texture2D{},
                .ClearType = RHI::ERTClearType::Color,
                .Format    = ResourceResolver::GetSwapchainFormat(),
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