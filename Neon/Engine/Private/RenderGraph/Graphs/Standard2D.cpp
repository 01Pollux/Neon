#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Graphs/Standard.hpp>

//

#include <Renderer/Render/SpriteRenderer.hpp>
#include <Renderer/Render/MeshRenderer.hpp>

//

namespace Neon::RG
{
    void CreateStandard2DRenderGraph(
        Scene::Component::Camera& CameraComponent,
        const flecs::entity&      Camera)
    {
        /*  auto Graph   = CameraComponent.NewRenderGraph(Camera);
          auto Builder = Graph->Reset();

          auto& GBuffer = Builder.AddPass<GBufferPass>();
          {
              GBuffer.AttachRenderer<Renderer::SpriteRenderer>();
              GBuffer.AttachRenderer<Renderer::MeshRenderer>();
          }

  #ifndef NEON_DIST
          Builder.AddPass<DebugPass>(ResourceId("GBufferAlbedo"));
  #endif

          Builder.AddPass<CopyToTexturePass>(
              CopyToTexturePass::CopyToTextureData{
                  .ViewName    = "GBufferAlbedoToFinal",
                  .Source      = ResourceId("GBufferAlbedo"),
                  .Destination = ResourceResolver::GetOutputImage() });

          Builder.Build();*/
    }
} // namespace Neon::RG